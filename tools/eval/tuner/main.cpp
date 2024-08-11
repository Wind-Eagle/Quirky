#include <fstream>
#include <memory>

#include "../../../src/util/io.h"
#include "reader.h"
#include "dataset.h"

void PrintHelp() {
    q_util::Print(
        "Quirky eval tuner is a tool that constructs eval psq and model weights"
        "using a set of games. Usage:\n"
        "-d: file with dataset\n"
        "-j: number of additional threads (default is 1)\n");
}

struct ExecutableArguments {
    std::string dataset_path;
    size_t number_of_threads = 1;
};

void Make(const ExecutableArguments& args) {
    Reader reader(args.dataset_path);
    std::unique_ptr<Dataset> dataset = std::make_unique<Dataset>();
    dataset->Load(reader);
}

int main(int argc, char* argv[]) {
    ExecutableArguments executable_arguments;
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
        } else {
            q_util::ExitWithError(QuirkyError::UnexpectedArgument);
        }
    }
    if (executable_arguments.dataset_path.empty()) {
        q_util::ExitWithError(QuirkyError::ParseError);
    }
    Make(executable_arguments);
}
