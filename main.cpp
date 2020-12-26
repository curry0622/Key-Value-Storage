#include<bits/stdc++.h>
// #include<io.h>
#include<errno.h>
#include<sys/stat.h>
using namespace std;

// function: return output file name
string getOutputFileName(const string& inputFileName) {
    string outputFileName = "";
    size_t pos = inputFileName.find_last_of("/\\");
    outputFileName = inputFileName.substr(pos + 1);
    pos = outputFileName.find(".");
    outputFileName = outputFileName.substr(0, pos) + ".output";
    return outputFileName;
}

// function: return a long long int bit format
string longToBit(long long int value) {
    string bitStr = "";
    for(int i = 0; i < 64; i++)
        bitStr += to_string((value >> i) & 1);
    reverse(bitStr.begin(), bitStr.end());
    return bitStr;
}

// function: return different section of a 64-bit string
string cutBitStr(string bitStr, int section) {
    string subBitStr = "";
    switch(section) {
        case 0:
            for(int i = 0; i <= 15; i++)
                subBitStr += bitStr.at(i);
            break;
        case 1:
            for(int i = 16; i <= 31; i++)
                subBitStr += bitStr.at(i);
            break;
        case 2:
            for(int i = 32; i <= 47; i++)
                subBitStr += bitStr.at(i);
            break;
        case 3:
            for(int i = 48; i <= 63; i++)
                subBitStr += bitStr.at(i);
            break;
    }
    return subBitStr;
}

// function: return the directory which this key points to
string keyToDir(long long int key) {
    string dir = "./storage";
    string bitStr = longToBit(key);
    for(int i = 0; i < 3; i++)
        dir += "/" + cutBitStr(bitStr, i);
    return dir;
}

// function: return the whole path which this key point to
string keyToPath(long long int key) {
    string path = "./storage";
    string bitStr = longToBit(key);
    for(int i = 0; i < 3; i++)
        path += "/" + cutBitStr(bitStr, i);
    path += "/" + cutBitStr(bitStr, 3) + ".txt";
    return path;
}

// function: find specific key in disk
string findKeyInDisk(long long int key) {
    string path = keyToPath(key);
    string value = "EMPTY";
    ifstream fin(path);
    if(fin.is_open())
        fin >> value;
    return value;
}

// function: output result to file
bool outputResultToFile(const vector<string>& vec, string fileName, bool last = false) {
    if(vec.size() > 0) {
        if(!last)
            cout << "> Buffer full, outputting results to " << fileName << '\n';
        else
            cout << "> Outputting results to " << fileName << '\n';
        FILE* fp;
        fp = fopen(fileName.c_str(), "a");
        if(fp != NULL) {
            for(long long int i = 0; i < vec.size(); i++) {
                if(i != vec.size() - 1 || !last) {
                    fprintf(fp, "%s\n", vec.at(i).c_str());
                } else {
                    fprintf(fp, "%s", vec.at(i).c_str());
                }
            }
        } else {
            cout << "> Failed to open " << fileName << '\n';
        }
        fclose(fp);
        return true;
    }
    cout << "> No content to output\n";
    return false;
}

// function: output specific key value to disk, return true if success, false if failed
bool renewKeyValue(long long int key, string value) {
    string bitStr = longToBit(key);
    string path = "./storage";
    for(int i = 0; i < 3; i++) {
        path += "/" + cutBitStr(bitStr, i);
        mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
    path += "/" + cutBitStr(bitStr, 3) + ".txt";
    FILE* fp;
    fp = fopen(path.c_str(), "w");
    if(fp != NULL) {
        fprintf(fp, "%s", value.c_str());
    } else {
        cout << "> Failed to open " << path << '\n';
        cout << "> errno = " << errno << '\n';
        return false;
    }
    fclose(fp);
    return true;
}

// function: merge memory and all.txt content, and output to all.txt
bool renewAllTxt(string dir, const map<long long int, string>& mp) {
    string path = dir + "/all.txt";
    FILE* fp;
    fp = fopen(path.c_str(), "w");
    if(fp != NULL) {
        int cntLine = 0;
        for(auto iter = mp.begin(); iter != mp.end(); iter++, cntLine++) {
            if(cntLine != mp.size() - 1) {
                fprintf(fp, "%lld %s\n", iter -> first, (iter -> second).c_str());
            } else {
                fprintf(fp, "%lld %s", iter -> first, (iter -> second).c_str());
            }
        }
        fclose(fp);
    } else {
        cout << "> Failed to open " << path << '\n';
        fclose(fp);
        return false;
    }
}

// function: renew disk, data.txt and all.txt
bool renewDisk(const map<long long int, pair<string, bool> >& mp) {
    // container to store new data in each directory
    map<string, map<long long int, string> > dirMap;
    // output to data.txt
    // cout << "> Outputting to data.txt\n";
    for(const auto& m: mp) {
        // first check if it's new data
        if((m.second).second) {
            // new data from PUT
            renewKeyValue(m.first, (m.second).first);
            // new data to put into corresponding all.txt
            dirMap[keyToDir(m.first)][m.first] = (m.second).first;
        }
    }
    // output to all.txt
    // cout << "> Outputting to all.txt\n";
    for(auto& dir: dirMap) {
        string path = (dir.first) + "/all.txt";
        ifstream getDirContent(path);
        if(getDirContent.is_open()) {
            // all.txt exists, first load them out
            while(!getDirContent.eof()) {
                long long int key;
                string value;
                getDirContent >> key >> value;
                if((dir.second).find(key) == (dir.second).end()) {
                    // not found in container of data of this all.txt, load it in
                    (dir.second)[key] = value;
                }
            }
        }
        renewAllTxt(dir.first, dir.second);
    }
}

// function: push back output, output all content to disk if it reaches max size
bool pushBackOutput(vector<string>& output, string result, string fileName) {
    int maxSize = 5000000;
    if(output.size() + 1 > maxSize) {
        // buffer overflow
        outputResultToFile(output, fileName);
        output.clear();
        vector<string>().swap(output);
    }
    output.push_back(result);
}

// function: insert data to map, output some content to disk if it reaches max size
bool insertMemory(map<long long int, pair<string, bool> >& memory, long long int key, string value, bool newDataOrNot, set<string>& opened) {
    // memory max size
    int maxSize = 5000000;
    if(memory.size() + 1 > maxSize) {
        // buffer overflow
        // cout << "> Memory buffer overflow, freeing some memory, renewing disk\n";
        // container to store new data in each directory
        map<string, map<long long int, string> > dirMap;
        // cout << "> Outputting to data.txt\n";
        // int newData = 0;
        for(int i = 0; i < 524288; i++) {
            auto memoryBegin = memory.begin();
            if((memoryBegin -> second).second) {
                // newData++;
                // if memory first data is new
                renewKeyValue(memoryBegin -> first, (memoryBegin -> second).first);
                dirMap[keyToDir(memoryBegin -> first)][memoryBegin -> first] = (memoryBegin -> second).first;
            }
            // erase memory first data
            memory.erase(memoryBegin);
        }
        // cout << "> New data: " << newData << " , Old data: " << 524288 - newData << '\n';
        // cout << "> Outputting to all.txt\n";
        for(auto& dir: dirMap) {
            string path = (dir.first) + "/all.txt";
            auto findFile = opened.find(path);
            if(findFile != opened.end()) {
                opened.erase(findFile);
            }
            ifstream getDirContent(path);
            if(getDirContent.is_open()) {
                // all.txt exists, first load them out
                while(!getDirContent.eof()) {
                    long long int key;
                    string value;
                    getDirContent >> key >> value;
                    if((dir.second).find(key) == (dir.second).end()) {
                        // not found in container of new data of this all.txt, load it in
                        (dir.second)[key] = value;
                    }
                }
            }
            renewAllTxt(dir.first, dir.second);
        }
    }
    // insert memory
    memory[key] = make_pair(value, newDataOrNot);
}

// global variable: container for memory
map<long long int, pair<string, bool> > memoryMap;

// global variable: container of output result
vector<string> outputVec;

// global variable: container of opened files
set<string> openedFiles;

// global variables: record some detailed information
long long int totalHit = 0;
long long int totalMiss = 0;
long long int getHit = 0;
long long int getMiss = 0;
long long int scanHit = 0;
long long int scanMiss = 0;
long long int putTime = 0;
long long int getTime = 0;
long long int scanTime = 0;


int main(int argc, char *argv[]) {

	// check if enter input file path as an argument
	if(argc < 2) {
		cout << "> Please enter input file's path !\n";
		return -1;
	}

    // start the clock
    long long int startClk = clock();

    // get file name
    string inputFileName = argv[1];
    string outputFileName = "./" + getOutputFileName(inputFileName);

    // print file information
    cout << "> Input file: " << inputFileName << '\n';
	cout << "> Output file: " << outputFileName << '\n';

	// create ./storage
	mkdir("./storage", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    // open input file
    ifstream getInput;
    getInput.open(inputFileName);

    // processing inputs
    int cntInstr = 0;
    if(getInput.is_open()) {
	cout << "> Reading inputs...\n";
        while(!getInput.eof()) {
            cntInstr++;
            if(cntInstr % 10000 == 0) {
                cout << "> On " << cntInstr << "th instruction\n";
            }
            // get mode: PUT, GET, SCAN
            string mode;
            getInput >> mode;
            long long int getModeTime = clock();
            if(mode == "PUT") {
                // PUT [key] [value]
                long long int key;
                string value;
                getInput >> key >> value;
                // cout << "[" << cntInstr << "]: PUT " << key << '\n';
                insertMemory(memoryMap, key, value, true, openedFiles);
                putTime += clock() - getModeTime;
            } else if(mode == "GET") {
                // GET [key]
                long long int key;
                getInput >> key;
                // cout << "[" << cntInstr << "]: GET " << key << '\n';
                auto iter = memoryMap.find(key);
                if(iter != memoryMap.end()) {
                    // key found in memory
                    pushBackOutput(outputVec, (iter -> second).first, outputFileName);
                    // hit
                    getHit++;
                    totalHit++;
                } else {
                    // key not found in memory, go to disk
                    string result = findKeyInDisk(key);
                    // push back output
                    pushBackOutput(outputVec, result, outputFileName);
                    // update memory, EMPTY possible
                    insertMemory(memoryMap, key, result, false, openedFiles);
                    // miss
                    getMiss++;
                    totalMiss++;
                }
                getTime += clock() - getModeTime;
            } else {
                // SCAN [key1] [key2]
                long long int key1, key2;
                getInput >> key1 >> key2;
                // cout << "[" << cntInstr << "]: SCAN " << key1 << " " << key2 << '\n';
                // iterate all keys
                for(long long int keyi = key1; keyi <= key2; keyi++) {
                    auto iter = memoryMap.find(keyi);
                    if(iter != memoryMap.end()) {
                        // key found in memory
                        pushBackOutput(outputVec, (iter -> second).first, outputFileName);
                        // hit
                        scanHit++;
                        totalHit++;
                    } else {
                        // key not found in memory, go to disk
                        string path = keyToDir(keyi) + "/all.txt";
                        if(openedFiles.find(path) != openedFiles.end()) {
                            // file already loaded, but not found in map
                            pushBackOutput(outputVec, "EMPTY", outputFileName);
                            insertMemory(memoryMap, keyi, "EMPTY", false, openedFiles);
                            // hit
                            scanHit++;
                            totalHit++;
                        } else {
                            // file unloaded
                            openedFiles.insert(path);
                            ifstream diskIn(path);
                            if(diskIn.is_open()) {
                                // file exists in disk
                                string foundValue = "EMPTY";
                                while(!diskIn.eof()) {
                                    long long int key;
                                    string value;
                                    diskIn >> key >> value;
                                    if(memoryMap.find(key) == memoryMap.end()) {
                                        // disk data not in memory, load it into memory
                                        insertMemory(memoryMap, key, value, false, openedFiles);
                                    }
                                    if(key == keyi) {
                                        // found in disk
                                        foundValue = value;
                                    }
                                }
                                pushBackOutput(outputVec, foundValue, outputFileName);
                                insertMemory(memoryMap, keyi, foundValue, false, openedFiles);
                            } else {
                                // file doesn't exist in disk
                                long long int startKey = keyi / 65536 * 65536;
                                for(long long int emptyKey = startKey; emptyKey < startKey + 65536; emptyKey++) {
                                    if(memoryMap.find(emptyKey) == memoryMap.end()) {
                                        insertMemory(memoryMap, emptyKey, "EMPTY", false, openedFiles);
                                    }
                                }
                                pushBackOutput(outputVec, "EMPTY", outputFileName);
                            }
                            // miss
                            scanMiss++;
                            totalMiss++;
                        }
                    }
                }
                scanTime += clock() - getModeTime;
            }
        }
        // output to file
        outputResultToFile(outputVec, outputFileName, true);

        // renew disk
        renewDisk(memoryMap);

    } else {
        // Failed to open input file
        cout << "> Failed to open " << inputFileName << '\n';
        return -1;
    }
    // print information
    cout << ">> GET hit: " << getHit << '\n';
    cout << ">> GET miss: " << getMiss << '\n';
    cout << ">> SCAN hit: " << scanHit << '\n';
    cout << ">> SCAN miss: " << scanMiss << '\n';
    cout << ">> Total hit: " << totalHit << '\n';
    cout << ">> Total miss: " << totalMiss << '\n';
    cout << ">> Hit rate: " << double(totalHit) / double(totalHit + totalMiss) << '\n';
    cout << ">> Total time: " << (clock() - startClk) / double(CLOCKS_PER_SEC) << "sec\n";
}

