#include <cstddef>
#include <cstdlib>

#include "../../src/util/error.h"
#include "../../src/util/io.h"
#include "reader.h"
#include "writer.h"

void PrintHelp() {
    q_util::Print(
        "Quirky eval model sampler is a tool that can transform labeled fens set"
        "into a dataset for Quirky eval model learner. Usage:\n",
        "--help: print help\n", "-i [path to file] - path to the dataset file\n",
        "-i [path to file] - file with raw dataset\n",
        "-o [path to directory] - directory with datasets\n",
        "-r [float] - ratio of test dataset elements");
}

struct SamplerArguments {
    std::string_view input_file;
    std::string_view out_dir;
    float test_ratio = 0.1;
};

void Make(const SamplerArguments& args) {
    std::ifstream in(args.input_file.data());
    size_t pos = 1;
    std::ofstream train_out(std::string(args.out_dir) + "/train.qds", std::ios::binary);
    std::ofstream test_out(std::string(args.out_dir) + "/test.qds", std::ios::binary);
    while (true) {
        PositionSet game_set = ReadPositions(in, (1 << 20));
        if (game_set.positions.empty()) {
            break;
        }
        std::string out_file_name = std::to_string(pos++);
        if (out_file_name.size() == 1) {
            out_file_name .insert(out_file_name.begin(), '0');
        }
        WriteBoardsToCSV(game_set, train_out, test_out, args.test_ratio);
    }
}

int main(int argc, char* argv[]) {
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
            sampler_arguments.out_dir = std::string_view(argv[i + 1]);
        } else if (std::string(argv[i]) == "-r") {
            sampler_arguments.test_ratio = std::stof(argv[i + 1]);
        }
        else {
            q_util::ExitWithError(QuirkyError::UnexpectedArgument);
        }
    }
    if (sampler_arguments.input_file.empty() || sampler_arguments.out_dir.empty()) {
        q_util::ExitWithError(QuirkyError::ParseError);
    }
    Make(sampler_arguments);
}