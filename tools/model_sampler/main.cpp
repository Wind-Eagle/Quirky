#include <cstddef>
#include <map>

#include "../../src/util/error.h"
#include "../../src/util/io.h"
#include "reader.h"
#include "writer.h"

void PrintHelp() {
    q_util::Print(
        "Quirky eval model sampler is a tool that can transform SGS games set"
        "into a dataset for Quirky eval model learner. Usage:\n",
        "--help: print help\n", "-i [path to file] - path to the SGS file\n",
        "-o [path to file] - path to the dataset csv file\n"
        "-s [positive integer] - size of file batch (default is 2048)");
}

struct SamplerArguments {
    std::string_view input_file;
    std::string_view output_file;
    size_t batch_size = 2048;
};

void Make(const SamplerArguments& args) {
    std::ifstream in(args.input_file.data());
    size_t pos = 1;
    while (true) {
        GameSet game_set = ReadGames(args.input_file, in, args.batch_size);
        if (game_set.games.empty()) {
            break;
        }
        std::string out_file_name = std::to_string(pos++);
        if (out_file_name.size() == 1) {
            out_file_name = "0" + out_file_name;
        }
        std::ofstream out(std::string(args.output_file) + "/" + out_file_name + ".csv");
        WriteBoardsToCSV(std::move(game_set), out);
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
            sampler_arguments.output_file = std::string_view(argv[i + 1]);
        } else if (std::string(argv[i]) == "-s") {
            sampler_arguments.batch_size = std::stoi(argv[i + 1]);
        } else {
            q_util::ExitWithError(QuirkyError::UnexpectedArgument);
        }
    }
    if (sampler_arguments.input_file.empty() || sampler_arguments.output_file.empty()) {
        q_util::ExitWithError(QuirkyError::ParseError);
    }
    Make(sampler_arguments);
}