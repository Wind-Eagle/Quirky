package main

import (
	"bufio"
	"context"
	"fmt"
	"math/rand/v2"
	"os"
	"strings"
	"time"

	"github.com/alex65536/go-chess/chess"
	"github.com/alex65536/go-chess/uci"
	"github.com/alex65536/go-chess/util/maybe"
	"github.com/cheggaaa/pb/v3"
	"golang.org/x/sync/errgroup"
)

type Pos struct {
	fen      string
	score    float32
	bestMove string
	wdl      uci.WDL
	target   float32
}

func getResult(ctx context.Context, engine *uci.Engine, fen string, ms int) (Pos, error, bool) {
	ctx, cancel := context.WithDeadline(ctx, time.Now().Add(5000*time.Millisecond))
	defer cancel()
	game, err := chess.NewGameWithFEN(fen)
	if err != nil {
		return Pos{}, fmt.Errorf("new game: %w", err), false
	}
	if game.CalcOutcome().IsFinished() {
		return Pos{}, nil, false
	}
	if err := engine.SetPosition(ctx, game); err != nil {
		return Pos{}, fmt.Errorf("set position: %w", err), false
	}
	search, err := engine.Go(ctx, uci.GoOptions{
		Movetime: maybe.Some(time.Duration(ms) * time.Millisecond),
	}, nil)
	if err != nil {
		return Pos{}, fmt.Errorf("go: %w", err), false
	}
	if err := search.Wait(ctx); err != nil {
		return Pos{}, fmt.Errorf("wait: %q: %w", fen, err), false
	}
	bestMove, err := search.BestMove()
	if err != nil {
		return Pos{}, fmt.Errorf("no best move"), false
	}
	score, ok := search.Status().Score.TryGet()
	if !ok {
		return Pos{}, fmt.Errorf("no score"), false
	}
	wdl, ok := search.Status().WDL.TryGet()
	if !ok {
		return Pos{}, fmt.Errorf("no wdl"), false
	}
	if game.CurBoard().Side() == chess.ColorBlack {
		wdl.Win, wdl.Loss = wdl.Loss, wdl.Win
	}
	wdlScore := 0.0
	num := rand.Float64()
	if num < wdl.Win {
		wdlScore = 1
	} else if num < wdl.Win+wdl.Draw {
		wdlScore = 0.5
	} else {
		wdlScore = 0
	}
	scoreScore := 0.0
	val, isMate := score.Mate()
	sval, isScore := score.Centipawns()
	if isMate {
		if val < 0 {
			scoreScore = -30000
		} else {
			scoreScore = 30000
		}
	} else if isScore {
		scoreScore = float64(sval)
	} else {
		panic("never happens")
	}
	if game.CurBoard().Side() == chess.ColorBlack {
		scoreScore *= -1
	}
	return Pos{fen: fen, bestMove: bestMove.UCI(), score: float32(scoreScore), wdl: wdl, target: float32(wdlScore)}, nil, true
}

func prepareTeacherEngine(ctx context.Context, name string) *uci.Engine {
	engine, err := uci.NewEasyEngine(ctx, uci.EasyEngineOptions{
		Name:            name,
		WaitInitialized: true,
	})
	if err != nil {
		panic(err)
	}
	err = engine.SetOption(ctx, "UCI_ShowWDL", uci.OptValueBool(true))
	if err != nil {
		panic(err)
	}
	return engine
}

func countLines(filePath string) (int, error) {
	file, err := os.Open(filePath)
	if err != nil {
		return 0, err
	}
	defer file.Close()

	scanner := bufio.NewScanner(file)
	lineCount := 0

	for scanner.Scan() {
		lineCount++
	}

	if err := scanner.Err(); err != nil {
		return 0, err
	}

	return lineCount, nil
}

func LabelFens(input string, output string, settings Settings) {
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	g, ctx := errgroup.WithContext(ctx)
	g2, _ := errgroup.WithContext(ctx)

	file, _ := os.Create(output)
	defer file.Close()

	ch := make(chan string)
	pch := make(chan Pos)
	stop := make(chan struct{})

	g2.Go(func() error {
		for {
			var pos Pos
			select {
			case <-stop:
				return nil
			case pos = <-pch:
			}
			fmt.Fprintf(file, "%v,%v,%v,%v,%v,%v,%v\n", pos.fen, pos.score, pos.bestMove, pos.wdl.Win, pos.wdl.Draw, pos.wdl.Loss, pos.target)
		}
	})

	numOfChunks, err := countLines(input)
	if err != nil {
		panic(err)
	}
	tmpl := `{{string . "prefix"}} {{counters . }} {{bar . }} {{percent . }} ({{speed . }} fens/sec)`
	bar := pb.ProgressBarTemplate(tmpl).Start(numOfChunks)
	for range settings.Cores {
		g.Go(func() error {
			engine := prepareTeacherEngine(ctx, settings.TeacherEngineName)
			defer engine.Close()
			for {
				fen, ok := <-ch
				if !ok {
					break
				}
				pos, err, ok := getResult(ctx, engine, fen, settings.LabelMovetime)
				if err != nil {
					fmt.Println("error in getting dataset line:", err)
					continue
				}
				if !ok {
					continue
				}
				pch <- pos
				bar.Increment()
			}
			return nil
		})
	}
	inputFile, _ := os.Open(input)
	defer inputFile.Close()
	scanner := bufio.NewScanner(inputFile)
	num := 0

	for scanner.Scan() {
		line := scanner.Text()
		num++
		ch <- strings.TrimSpace(line)
		if num == numOfChunks {
			break
		}
	}
	close(ch)
	if err := g.Wait(); err != nil {
		fmt.Printf("Error occurred: %v\n", err)
	}
	close(stop)
	if err := g2.Wait(); err != nil {
		fmt.Printf("Error occurred: %v\n", err)
	}
	bar.Finish().Current()
}
