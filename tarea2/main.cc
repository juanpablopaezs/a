#include <global.hh>
#include <getopt.h>

std::string removePunctuation(const std::string& word) {
    std::string result;
    for (char c : word) {
        if (std::isalpha(c)) {
            result += c;
        }
    }
    return result;
}
// Función para dividir un texto en palabras
std::vector<std::string> splitText(const std::string &text) {
    std::vector<std::string> words;
    std::string word;
    std::istringstream iss(text);
    while (iss >> word) {
        for (char &c : word) {
            c = std::tolower(c);
        }
        word = removePunctuation(word);
        words.push_back(word);
    }
    return words;
}
// Función para dividir un texto en palabras en multi-thread
void processText(const std::vector<std::string>& lines, std::map<std::string, int>& wordHistogram, std::mutex& mtx) {
    for (const std::string& line : lines) {
        std::vector<std::string> words = splitText(line);
        for (const std::string& word : words) {
            std::lock_guard<std::mutex> lock(mtx);
            wordHistogram[word]++;
        }
    }
}

int main(int argc, char* argv[]) {
    int numThreads = 1; 
    std::string fileName;
    std::vector<std::string> textInMemory;

    static struct option long_options[] = {
        {"threads", required_argument, 0, 't'},
        {"file", required_argument, 0, 'f'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int option;
    while ((option = getopt_long(argc, argv, "t:f:h", long_options, NULL)) != -1) {
        switch (option) {
            case 't':
                numThreads = std::atoi(optarg);
                break;
            case 'f':
                fileName = optarg;
                break;
            case 'h':
                std::cout << "Modo de Uso: " << argv[0] << " --threads N --file FILENAME [--help]" << std::endl;
                std::cout << "--threads: cantidad de threads a utilizar. Si es 1, entonces ejecuta la versión secuencial." << std::endl;
                std::cout << "--file: archivo a procesar." << std::endl;
                std::cout << "--help: muestra este mensaje y termina." << std::endl;
                return EXIT_SUCCESS;
            default:
                return EXIT_FAILURE;
        }
    }

    if (fileName.empty() || numThreads <= 0) {
                std::cout << "Modo de Uso: " << argv[0] << " --threads N --file FILENAME [--help]" << std::endl;
                std::cout << "--threads: cantidad de threads a utilizar. Si es 1, entonces ejecuta la versión secuencial." << std::endl;
                std::cout << "--file: archivo a procesar." << std::endl;
                std::cout << "--help: muestra este mensaje y termina." << std::endl;
        return EXIT_FAILURE;
    }

    std::ifstream file(fileName);
    if (!file) {
        std::cerr << "No se pudo abrir el archivo." << std::endl;
        return EXIT_FAILURE;
    }

    std::string line;
    while (std::getline(file, line)) {
        textInMemory.push_back(line);
    }
    file.close();

    std::map<std::string, int> wordHistogram;

    ////multi-thread o secuencial funciona en los 2 casos 

        std::vector<std::thread> threads;
        std::mutex mtx;

        int linesPerThread = textInMemory.size() / numThreads;
        for (int i = 0; i < numThreads; ++i) {
            int start = i * linesPerThread;
            int end = (i == numThreads - 1) ? textInMemory.size() : (i + 1) * linesPerThread;
            threads.emplace_back(processText, std::vector<std::string>(textInMemory.begin() + start, textInMemory.begin() + end), std::ref(wordHistogram), std::ref(mtx));
        }

       ///////unir los thread 
        for (std::thread& thread : threads) {
            thread.join();
        }

    // Mostrar el histograma de palabras
    for (const auto& entry : wordHistogram) {
        std::cout << entry.first << ": " << entry.second << std::endl;
    }

    return EXIT_SUCCESS;
}