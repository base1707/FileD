#include <iostream>
#include <fstream>
#include <filesystem>

#include <bitset>
#include <unordered_map>
#include <set>

#include <random>
#include <ctime>

namespace Print
{
    template <typename T>
    void Message(T&& message)
    {
        if (std::cout.good())
            std::cout << "[#] " << message << std::endl;
    }
    
    template <typename T>
    void Error(T&& message)
    {
        if (std::cout.good())
            std::cout << "[!] " << message << std::endl;
    }
}

namespace Random
{
    class CPRNG final
    {
    private:
        // Using Mersenne Twister for generate unique numbers
        std::mt19937 engine;
        
    public:
        CPRNG()
        {
            // Window & MinGW fix
            // (https://ps-group.github.io/cxx/cxx_random)
            engine.seed(unsigned(std::time(nullptr)));
        }
        
        // Get a random value in [ lowBound ; highBound ]
        std::size_t GetRandom(std::size_t lowBound, std::size_t highBound)
        {
            if (lowBound >= highBound)
            {
                Print::Error("Incorrect bounds selected!");
                return 0;
            }
            
            std::uniform_int_distribution<std::size_t> dist(lowBound, highBound);
            return dist(engine);
        }
    };
}

namespace Software
{
    using HASH_INPUT_TYPE = std::string;
    using HASH_OUTPUT_TYPE = std::size_t;
    using HASH_MAP = std::unordered_map<HASH_OUTPUT_TYPE, std::filesystem::path>;
    
    HASH_OUTPUT_TYPE GetHashFromInput(HASH_INPUT_TYPE buffer)
    {
        // Not good: using default STL::hash API
        return std::hash<HASH_INPUT_TYPE>{}(buffer);
    }
    
    HASH_OUTPUT_TYPE GetHashFromFile(const std::filesystem::path& p)
    {
        if (p.empty() || !std::filesystem::is_regular_file(p))
        {
            Print::Error("Incorrect file path!");
            return 0;
        }
        
        // Try to open file in binary
        std::ifstream file { p.c_str(), std::ios::in | std::ios::binary };
        if (!file.is_open())
        {
            Print::Error("Incorrect file details!");
            return 0;
        }
        
        HASH_INPUT_TYPE buffer;
        
        // Update buffer size
        file.seekg(0, std::ios::end);
        buffer.reserve(file.tellg());
        
        // Read file data (binary)
        file.seekg(0, std::ios::beg);
        buffer.assign(std::istreambuf_iterator<char>{file}, {});
        
        // Close file thread
        file.close();
        
        // Get hash from buffer
        return Software::GetHashFromInput(buffer);
    }
    
    int EntryPoint(const std::filesystem::path& dirPath, bool renameFiles, bool removeDuplicates)
    {
        if (dirPath.empty())
        {
            Print::Error("Path is empty!");
            return 2;
        }
        
        if (!std::filesystem::is_directory(dirPath))
        {
            Print::Error("Selected path isn't a directory!");
            return 2;
        }
        
        Print::Message("Duplicates finding started...");
        HASH_MAP files;
        std::string buffer;
        std::size_t counter{};
        
        // Step 1. Caching, finding duplicates
        for (const auto& entry : std::filesystem::recursive_directory_iterator{ dirPath })
        {
            // Skip directories, FIFO, symlinks and other types
            const std::filesystem::path& p{ entry.path() };
            if (!std::filesystem::is_regular_file(p))
                continue;
            
            // Try to emplace hash in map
            const auto& [it, status] = files.try_emplace(Software::GetHashFromFile(p), p);
            
            // If file already pushed - a duplicate in face of us
            if (!status)
            {
                ++counter;
                
                // Prepare output
                buffer = std::string("File [") + p.c_str() + std::string("] is duplicate of \n\t[");
                buffer += it->second.c_str() + std::string("]");
                buffer += removeDuplicates ? std::string(", removing...") : std::string(", ignoring...");
                Print::Message(buffer);
                
                // If we have special permission - remove duplicate
                if (removeDuplicates)
                {
                    std::filesystem::remove(p);
                }
            }
        }
        
        buffer = std::to_string(counter) + std::string(" duplicates found!");
        Print::Message(buffer);
        
        // Step 2. Renaming.
        if (renameFiles)
        {
            Print::Message("Renaming started...");
            
            // Using Mersenne Twister for generate unique numbers
            Random::CPRNG randomGenerator;
            std::set<std::size_t> unique;
            std::size_t generatedValue{};
            
            // Iterate saved files
            for (const auto& it : files)
            {
                const auto& oldP = it.second;
                // User could delete the file
                if (!std::filesystem::is_regular_file(oldP))
                    continue;
                
                // Try to give new unique name for file
                do
                {
                    generatedValue = randomGenerator.GetRandom(1'000'000'000, 9'999'999'999);
                }
                while (unique.find(generatedValue) != unique.end());
                
                if (generatedValue == 0)
                {
                    Print::Error("Incorrect generatedValue (== 0), skipped!");
                    continue;
                }
                
                // Update unique values
                unique.insert(generatedValue);
                
                // Construct a new path: {DIRECTORY} + {VALUE} + {EXTENSION}
                std::filesystem::path newP { oldP.parent_path() };
                newP += "/" + std::to_string(generatedValue) + std::string(oldP.extension());
                std::filesystem::rename(oldP, newP);
                
                // Send message to user
                Print::Message(
                    "File [" + std::string(oldP) + "] renamed to \n\t[" + std::string(newP) + "]" 
                );
            }
        }
        
        // We updated file paths - information is outdated
        files.clear();
        
        // It's okay
        return 0;
    }
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        Print::Error("Too few arguments count! Use -h or --help for details.");
        return 1;
    }
    
    // [0 - rename, 1 - duplicates]
    std::bitset<2> flags{"00"};
    
    // Directory path
    std::string dirPath;
    
    // Handle console arguments
    std::string buffer;
    for (int i = 1; i < argc; ++i)
    {
        buffer = argv[i];
        if (buffer == "-h" || buffer == "--help")
        {
            Print::Message(
                "Command list:"
                "\n\t--help \t\t(-h) - guide"
                "\n\t--path \t\t(-p) - path to analyze"
                "\n\t--rename \t(-r) - rename files in directories"
                "\n\t--duplicates \t(-d) - delete finded duplicates"
            );
            return 0;
        }
        else if (buffer == "-r" || buffer == "--rename")
        {
            flags.set(0, true);
        }
        else if (buffer == "-d" || buffer == "--duplicates")
        {
            flags.set(1, true);
        }
        else if (buffer == "-p" || buffer == "--path")
        {
            if (i + 1 >= argc)
            {
                Print::Error("Incorrect argument: " + buffer);
                return 1;
            }
            dirPath = argv[i + 1];
            ++i;
        }
        else
        {
            Print::Error("Incorrect argument: " + buffer);
            return 1;
        }
    }
    
    // Get status
    return Software::EntryPoint(dirPath, flags[0], flags[1]);
}
