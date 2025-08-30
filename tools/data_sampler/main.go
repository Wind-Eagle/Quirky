package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"log"
	"math"
	"os"
)

func getEstimatedCount(gamesInBatch int, fensInGame int) int {
	return int(math.Round(float64(gamesInBatch) * float64(fensInGame) * float64(2) / float64(1.9)))
}

func getEstimatedTime(gamesInBatch int, fensInGame int, playMovetime int, labelMovetime int, cores int) float32 {
	first := float32(gamesInBatch) * float32(160) * float32(playMovetime) / float32(1000.0) / float32(cores)
	second := float32(getEstimatedCount(gamesInBatch, fensInGame)) * float32(labelMovetime) / float32(1000.0) / float32(cores)
	return first + second
}

func main() {
	var (
		configPath = flag.String("c", "", "config path")
	)

	flag.Parse()
	file, err := os.ReadFile(*configPath)
	if err != nil {
		log.Fatal(err)
	}

	var settings Settings
	err = json.Unmarshal(file, &settings)
	if err != nil {
		log.Fatal(err)
	}

	estimatedTime := getEstimatedTime(settings.GamesInBatch, settings.FensInGame, settings.PlayMovetime, settings.LabelMovetime, settings.Cores) * float32(settings.Batches)
	estimatedCount := getEstimatedCount(settings.GamesInBatch, settings.FensInGame) * settings.Batches
	fmt.Println("Estimated time: " + fmt.Sprintf("%.2f", estimatedTime) + " seconds")
	fmt.Println("Estimated fens: " + fmt.Sprintf("%d", estimatedCount))

	for num := range settings.Batches {
		fensFile := fmt.Sprintf("%d", num) + ".fen"
		intermediateFile := fmt.Sprintf("%d", num) + ".txt"
		datasetFile := fmt.Sprintf("%d", num) + ".raw"
		fmt.Println("Batch #" + fmt.Sprintf("%d", num) + ", getting FENs")
		GenFens(fensFile, settings)
		fmt.Println("Batch #" + fmt.Sprintf("%d", num) + ", filtering FENs")
		FilterFens(fensFile, intermediateFile, settings.FilterBinaryName)
		err := os.Remove(fensFile)
		if err != nil {
			fmt.Println("error in deleting fens file:", err)
		}
		fmt.Println("Batch #" + fmt.Sprintf("%d", num) + ", labeling FENs")
		LabelFens(intermediateFile, datasetFile, settings)
		err = os.Remove(intermediateFile)
		if err != nil {
			fmt.Println("error in deleting intermediate file:", err)
		}
	}
}
