package main

import (
	"bufio"
	"context"
	"fmt"
	"math/rand/v2"
	"os"
	"time"

	"github.com/alex65536/go-chess/chess"
	"github.com/alex65536/go-chess/clock"
	"github.com/alex65536/go-chess/uci"
	"github.com/alex65536/go-chess/util/maybe"
	"github.com/cheggaaa/pb/v3"
	"golang.org/x/sync/errgroup"
)

func uciNewGame(ctx context.Context, engine *uci.Engine, timeout time.Duration) error {
	ctx, cancel := context.WithTimeout(ctx, timeout)
	defer cancel()
	if err := engine.UCINewGame(ctx, true); err != nil {
		return fmt.Errorf("ucinewgame: %w", err)
	}
	return nil
}

func getEngines(ctx context.Context, engineNames []string) (*uci.Engine, *uci.Engine) {
	whiteIndex := rand.Int32N(int32(len(engineNames)))
	blackIndex := rand.Int32N(int32(len(engineNames)))

	whiteEngineName := engineNames[whiteIndex]
	blackEngineName := engineNames[blackIndex]
	whiteEngine, err := uci.NewEasyEngine(ctx, uci.EasyEngineOptions{
		Name:            whiteEngineName,
		WaitInitialized: true,
	})
	if err != nil {
		panic(err)
	}
	blackEngine, err := uci.NewEasyEngine(ctx, uci.EasyEngineOptions{
		Name:            blackEngineName,
		WaitInitialized: true,
	})
	if err != nil {
		panic(err)
	}
	return whiteEngine, blackEngine
}

func takeRandom(src []string, num int) []string {
	if len(src) <= num {
		return src
	}
	rand.Shuffle(len(src), func(i, j int) {
		src[i], src[j] = src[j], src[i]
	})
	return src[:num]
}

func playGame(ctx context.Context, whiteEngine *uci.Engine, blackEngine *uci.Engine, fen string, ms int, num int) ([]string, error, bool) {
	control, err := clock.ControlFromString("40/60+0.5")
	if err != nil {
		panic(err)
	}

	engines := [chess.ColorMax]*uci.Engine{whiteEngine, blackEngine}

	for c := range chess.ColorMax {
		if err := uciNewGame(ctx, engines[c], 3*time.Second); err != nil {
			panic(fmt.Errorf("ucinewgame %v: %w", c.LongString(), err))
		}
	}

	board, err := chess.BoardFromFEN(fen)
	if err != nil {
		panic("invalid opening")
	}

	game := clock.NewGame(
		chess.NewGameWithPosition(board),
		maybe.Some(control),
		clock.GameOptions{
			OutcomeFilter: maybe.Some(chess.VerdictFilterRelaxed),
		},
	)

	fensWhite := make([]string, 0)
	fensBlack := make([]string, 0)
	infoConsumerWhite := uci.InfoConsumer(func(search *uci.Search, info uci.Info) {
		if info.String.IsSome() {
			fensWhite = append(fensWhite, info.String.Get())
		}
	})
	infoConsumerBlack := uci.InfoConsumer(func(search *uci.Search, info uci.Info) {
		if info.String.IsSome() {
			fensBlack = append(fensBlack, info.String.Get())
		}
	})

	for !game.IsFinished() {
		curSide := game.CurSide()
		engine := engines[curSide]

		if err := func() error {
			deadline, ok := game.Deadline()
			if !ok {
				panic("no time control")
			}

			ctx, cancel := context.WithDeadline(ctx, deadline.Add(1*time.Millisecond))
			defer cancel()

			if err := engine.SetPosition(ctx, game.Inner()); err != nil {
				return fmt.Errorf("set position: %w", err)
			}

			var infoConsumer uci.InfoConsumer
			if game.CurBoard().Side() == chess.ColorWhite {
				infoConsumer = infoConsumerWhite
			} else {
				infoConsumer = infoConsumerBlack
			}
			search, err := engine.Go(ctx, uci.GoOptions{
				Movetime: maybe.Pack(time.Duration(ms)*time.Millisecond, true),
			}, infoConsumer)
			if err != nil {
				return fmt.Errorf("go: %w", err)
			}

			if err := search.Wait(ctx); err != nil {
				return fmt.Errorf("wait: %w", err)
			}

			bestMove, err := search.BestMove()
			if err != nil {
				return fmt.Errorf("best move: %w", err)
			}

			if err := game.Push(bestMove); err != nil {
				return fmt.Errorf("add move: %w", err)
			}

			return nil
		}(); err != nil {
			fmt.Printf("engine error: %v\n", err)
			_ = game.Finish(chess.MustWinOutcome(chess.VerdictEngineError, curSide.Inv()))
			return fensWhite, err, false
		}
	}
	fensWhite = takeRandom(fensWhite, num)
	fensBlack = takeRandom(fensBlack, num)
	fens := append(fensWhite, fensBlack...)
	fens = takeRandom(fens, num*2)
	return fens, nil, true
}

func readLines(path string) ([]string, error) {
	file, err := os.Open(path)
	if err != nil {
		return nil, err
	}
	defer file.Close()

	var lines []string
	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		lines = append(lines, scanner.Text())
	}
	return lines, scanner.Err()
}

func getOpenings(file string) []string {
	lines, err := readLines(file)
	if err != nil {
		panic(err)
	}
	return lines
}

func GenFens(output string, settings Settings) {
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	g, ctx := errgroup.WithContext(ctx)
	g2, _ := errgroup.WithContext(ctx)

	file, _ := os.Create(output)
	defer file.Close()

	openings := getOpenings(settings.OpeningBook)

	ch := make(chan struct{})
	pch := make(chan string)
	stop := make(chan struct{})

	g2.Go(func() error {
		for {
			var fen string
			select {
			case <-stop:
				return nil
			case fen = <-pch:
			}
			fmt.Fprintf(file, "%v\n", fen)
		}
	})

	numOfChunks := settings.GamesInBatch
	tmpl := `{{string . "prefix"}} {{counters . }} {{bar . }} {{percent . }} ({{speed . }} fens/sec)`
	bar := pb.ProgressBarTemplate(tmpl).Start(numOfChunks)
	for range settings.Cores {
		g.Go(func() error {
			for {
				whiteEngine, blackEngine := getEngines(ctx, settings.EngineNames)
				_, ok := <-ch
				if !ok {
					whiteEngine.Close()
					blackEngine.Close()
					break
				}
				randomIndex := rand.Int32N(int32(len(openings)))
				randomItem := openings[randomIndex]
				fens, err, ok := playGame(ctx, whiteEngine, blackEngine, randomItem, settings.PlayMovetime, settings.FensInGame)
				if err != nil {
					fmt.Println("error in getting dataset line:", err)
				}
				if !ok {
					whiteEngine.Close()
					blackEngine.Close()
					continue
				}
				for _, fen := range fens {
					pch <- fen
				}
				bar.Increment()
				whiteEngine.Close()
				blackEngine.Close()
			}
			return nil
		})
	}

	for range numOfChunks {
		ch <- struct{}{}
	}
	close(ch)
	if err := g.Wait(); err != nil {
		fmt.Printf("Error occurred: %v\n", err)
	} else {
		fmt.Println("All requests completed successfully")
	}
	close(stop)
	if err := g2.Wait(); err != nil {
		fmt.Printf("Error occurred: %v\n", err)
	} else {
		fmt.Println("All lines written successfully")
	}
	bar.Finish().Current()
}
