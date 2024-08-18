#include <fstream>
#include <memory>

#include "../../../src/util/io.h"
#include "dataset.h"
#include "learner.h"
#include "reader.h"

void PrintHelp() {
    q_util::Print(
        "Quirky eval tuner is a tool that constructs eval psq and model weights"
        "using a set of games. Usage:\n"
        "-d: file with dataset\n"
        "-j: number of additional threads (default is 1)\n"
        "-o: file with results\n"
        "-s: file with state");
}

struct ExecutableArguments {
    std::string dataset_path;
    std::string output_filename;
    std::string state_filename;
    size_t number_of_threads = 1;
};

void Make(const ExecutableArguments& args) {
    Reader reader(args.dataset_path);
    std::shared_ptr<Dataset> dataset = std::make_shared<Dataset>();
    dataset->Load(reader);

    LearnerParams learner_params{.dataset = dataset,
                                 .output_filename = args.output_filename,
                                 .state_filename = args.state_filename,
                                 .number_of_threads = args.number_of_threads,
                                 .channel_size = 64};
    TuneWeights(learner_params);
}

int main(int argc, char* argv[]) {
    ExecutableArguments executable_arguments{};
    if (argc <= 1) {
        PrintHelp();
        return 0;
    }
    for (size_t i = 1; i < static_cast<size_t>(argc); i += 2) {
        if (std::string(argv[1]) == "--help") {
            PrintHelp();
            return 0;
        }
        if (std::string(argv[i]) == "-d") {
            executable_arguments.dataset_path = argv[i + 1];
        } else if (std::string(argv[i]) == "-j") {
            executable_arguments.number_of_threads = std::stoi(argv[i + 1]);
            if (executable_arguments.number_of_threads == 0) {
                q_util::ExitWithError(QuirkyError::UnexpectedValue);
            }
        } else if (std::string(argv[i]) == "-o") {
            executable_arguments.output_filename = argv[i + 1];
        } else if (std::string(argv[i]) == "-s") {
            executable_arguments.state_filename = argv[i + 1];
        } else {
            q_util::ExitWithError(QuirkyError::UnexpectedArgument);
        }
    }
    if (executable_arguments.dataset_path.empty()) {
        q_util::ExitWithError(QuirkyError::ParseError);
    }
    if (executable_arguments.output_filename.empty()) {
        q_util::ExitWithError(QuirkyError::ParseError);
    }
    Make(executable_arguments);
}
