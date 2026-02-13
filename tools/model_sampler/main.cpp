#include <cstddef>
#include <cstdlib>
#include <fstream>

#include "../../src/util/error.h"
#include "../../src/util/io.h"
#include "reader.h"
#include "writer.h"

void PrintHelp() {
    q_util::Print(
        "Quirky eval model sampler is a tool that can transform labeled fens set"
        "into a dataset for Quirky eval model learner. Usage:\n",
        "--help: print help\n",
        "--input [path to file] - file with raw dataset\n",
        "--output [path to directory] - directory with datasets\n",
        "--test-ratio [float] - ratio of test dataset elements\n"
        "--preliminary-ratio [float] - ratio of preliminary dataset elements\n"
        "--preliminary-chunks-count [integer] - ratio of preliminary dataset elements\n"
        "--chunks-count [integer] - number of output chunks");
}

struct SamplerArguments {
    std::string_view input_file;
    std::string_view out_dir;
    float test_ratio = 0.1;
    float preliminary_ratio = 0.5;
    size_t preliminary_chunks_count = 2;
    size_t chunks_count = 2;
};

void Make(const SamplerArguments& args) {
    std::ifstream in(args.input_file.data());
    size_t pos = 1;
    OutputSources output_sources;

    output_sources.test_ratio = args.test_ratio;
    output_sources.preliminary_ratio = args.preliminary_ratio;
    output_sources.preliminary_chunks_count = args.preliminary_chunks_count;
    output_sources.chunks_count = args.chunks_count;

    for (size_t i = 0; i < args.preliminary_chunks_count; i++) {
         output_sources.preliminary_train_outs.emplace_back(std::string(args.out_dir) + "/preliminary_train_chunk_" + std::to_string(i + 1) + ".qds", std::ios::binary);
    }
    output_sources.preliminary_test_out = std::ofstream(std::string(args.out_dir) + "/preliminary_test.qds", std::ios::binary);

    for (size_t i = 0; i < args.chunks_count; i++) {
         output_sources.train_outs.emplace_back(std::string(args.out_dir) + "/train_chunk_" + std::to_string(i + 1) + ".qds", std::ios::binary);
    }
    output_sources.test_out = std::ofstream(std::string(args.out_dir) + "/test.qds", std::ios::binary);
    while (true) {
        PositionSet game_set = ReadPositions(in, (1 << 20));
        if (game_set.positions.empty()) {
            break;
        }
        std::string out_file_name = std::to_string(pos++);
        if (out_file_name.size() == 1) {
            out_file_name .insert(out_file_name.begin(), '0');
        }
        WriteBoardsToCSV(game_set, output_sources);
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
        if (std::string(argv[i]) == "--input") {
            sampler_arguments.input_file = std::string_view(argv[i + 1]);
        } else if (std::string(argv[i]) == "--output") {
            sampler_arguments.out_dir = std::string_view(argv[i + 1]);
        } else if (std::string(argv[i]) == "--test-ratio") {
            sampler_arguments.test_ratio = std::stof(argv[i + 1]);
        } else if (std::string(argv[i]) == "--preliminary-ratio") {
            sampler_arguments.preliminary_ratio = std::stof(argv[i + 1]);
        } else if (std::string(argv[i]) == "--preliminary-chunks-count") {
            sampler_arguments.preliminary_chunks_count = std::stoi(argv[i + 1]);
        } else if (std::string(argv[i]) == "--chunks-count") {
            sampler_arguments.chunks_count = std::stoi(argv[i + 1]);
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