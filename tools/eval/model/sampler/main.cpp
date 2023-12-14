#include "../../../../src/util/io.h"
#include "../../../../src/util/error.h"
#include "calcer.h"
#include "filter.h"
#include "reader.h"
#include "writer.h"

void PrintHelp() {
    q_util::Print(
        "Quirky eval model sampler is a tool that can transform SGS games set"
        "into a dataset for Quirky eval model learner. Usage:\n",
        "--help: print help\n",
        "-i [path to file] - path to the SGS file\n",
        "-o [path to file] - path to the dataset csv file"
    );
}

struct SamplerArguments {
    std::string_view input_file;
    std::string_view output_file;
};

void Make(const SamplerArguments& args) {
    GameSet game_set = ReadGames(args.input_file);
    BoardSetWithFeatures raw_boards = CalcFeatures(std::move(game_set));
    BoardSetWithFeatures boards = FilterBoards(std::move(raw_boards));
    WriteBoardsToCSV(std::move(boards), args.output_file);
}

int main(int argc, char *argv[]) {
    SamplerArguments sampler_arguments;
    if (argc <= 1) {
        PrintHelp();
        return 0;
    }
    for (size_t i = 1; i < static_cast<size_t>(argc); i += 2) {
        if (std::string(argv[1]) == "--help") {
            PrintHelp();
            return 0;
        }
        if (std::string(argv[i]) == "-i") {
            sampler_arguments.input_file = std::string_view(argv[i + 1]);
        } else if (std::string(argv[i]) == "-o") {
            sampler_arguments.output_file = std::string_view(argv[i + 1]);
        } else {
            q_util::ExitWithError(QuirkyError::UnexpectedArgument);
        }
    }
    Make(sampler_arguments);
}