package main

type Settings struct {
	GamesInBatch      int    `json:"gamesInBatch"`
	Batches           int    `json:"batches"`
	FensInGame        int    `json:"fensInGame"`
	Cores             int    `json:"cores"`
	EngineName        string `json:"engineName"`
	TeacherEngineName string `json:"teacherEngineName"`
	FilterBinaryName  string `json:"filterBinaryName"`
	PlayMovetime      int    `json:"playMovetime"`
	LabelMovetime     int    `json:"labelMovetime"`
	OpeningBook       string `json:"openingBook"`
}
