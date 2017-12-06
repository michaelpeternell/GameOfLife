//
//  main.cpp
//  GameOfLife
//
//  Created by Michael Peternell on 5.12.2017.
//  Copyright © 2017 Michael Peternell. All rights reserved.
//


#include "gol-config.h"
#include "Board.h"
#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <chrono>
#include <ctime>
#include <ratio>
#include <memory>
#include <cmath>
#include <cassert>

using namespace std;
using namespace std::chrono;


class Main
{
public:
    int run(int argc, char **argv);
    
    Board loadBoard(const std::string& path);
    void saveBoard(const Board& board, const std::string& path);
    
private:
    bool parseArguments(int argc, char **argv);
    
    void fail(string reason) { sayError(reason); exit(1); };
    void sayVerbose(string message) { if (arg_verbose) { logMessage(message); } };
    void sayWarning(string message) { logMessage("Warning: " + message); };
    void sayError(string message) { logMessage("Error: " + message); };
    
    void myAssert(bool condition, string reason) { if (!condition) { fail(reason); } };
    
    //
    // low level methods
    //
    
    void logMessage(string msg);
    string formatDuration(double duration);
    inline string formatDuration(duration<double> duration) { return formatDuration(duration.count()); };
    
    //
    // instance variables
    //
    
    // Time measurements
    high_resolution_clock::time_point timeStartInitPhase;
    high_resolution_clock::time_point timeStartKernelRunPhase;
    high_resolution_clock::time_point timeStartFinalizationPhase;
    high_resolution_clock::time_point timeFinished;
    
    // Command line arguments
    string arg_load;
    string arg_save;
    string arg_verify;
    string arg_mode;
    int arg_generations = -1;
    bool arg_measure = false;
    bool arg_verbose = false;
    
    // State
    Board board;
    Board verificationBoard;
};

int Main::run(int argc, char **argv)
{
    // Zeitmessung:
    //   Um die Zeit zu messen, teilen wir den Programmablauf in 3 Phasen ein:
    //   init time, kernel run time, finalize time.
    //     init time: parsing command line options, reading input file
    //     kernel run time: die eigentliche Berechnung
    //     finalize time: speichern des ergebnisses
    //   Wir verwenden die std::chrono::high_resolution_clock.
    
    // Kommandozeilen-Optionen:
    //   --load INPUTFILE
    //   --save OUTPUTFILE  (optional)
    //   --generations N    # Anzahl der Generationen die berechnet werden sollen. 0 <= N < (2^31-1)
    //   --measure          # Am Ende Zeitmessungen anzeigen
    //   --mode MODE        # Modus. Momentan wird nur "seq" unterstützt
    //       Modes:
    //          "seq" ... single threaded computation
    // Zusätzliche Optionen (nicht Teil der Aufgabe, aber zum Entwickeln praktisch)
    //  --verify VERIFYFILE # Vergleicht das Ergebnis mit einem File. Nützlich zum Testen
    //  --verbose
    
    bool ok;
    
    //
    // Init phase
    //
    
    timeStartInitPhase = high_resolution_clock::now();
    ok = parseArguments(argc, argv);
    if (!ok) {
        return 1;
    }
    
    //
    // Kernel run time
    //
    
    timeStartKernelRunPhase = high_resolution_clock::now();
    
    board.runSingleThreaded(arg_generations);
    
    //
    // Finalization time
    //
    
    timeStartFinalizationPhase = high_resolution_clock::now();
    bool verificationSuccess = true;
    bool saveSuccess = true;
    
    // Verification
    if (arg_verify != "") {
        bool same = board == verificationBoard;
        if (same) {
            sayVerbose("Verification successful!");
        }
        else {
            logMessage("Error: Verification failed: boards not equal");
            verificationSuccess = false;
        }
    }
    
    // Start of saving
    if (arg_save != "") {
        saveBoard(board, arg_save);
    }
    
    //
    // Print time measurements
    //
    
    timeFinished = high_resolution_clock::now();
    
    if (arg_measure) {
        duration<double> tdInitTime = duration_cast<duration<double>>(timeStartKernelRunPhase - timeStartInitPhase);
        duration<double> tdKernelRunTime = duration_cast<duration<double>>(timeStartFinalizationPhase - timeStartKernelRunPhase);
        duration<double> tdFinalizationTime = duration_cast<duration<double>>(timeFinished - timeStartFinalizationPhase);
        
        cout << formatDuration(tdInitTime) << "; " << formatDuration(tdKernelRunTime) << "; " << formatDuration(tdFinalizationTime) << ";\n";
    }
    
    return (verificationSuccess && saveSuccess) ? 0 : 1;
}

bool Main::parseArguments(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        string key = argv[i];
        string val;
        bool hasValue = false;
        if (i < argc - 1) {
            val = argv[i + 1];
            hasValue = true;
        }
        if (key == "--load") {
            arg_load = val;
            myAssert(hasValue, "Missing argument after "+key);
            i++;
        }
        else if (key == "--save") {
            arg_save = val;
            myAssert(hasValue, "Missing argument after " + key);
            i++;
        }
        else if (key == "--verify") {
            arg_verify = val;
            myAssert(hasValue, "Missing argument after " + key);
            i++;
        }
        else if (key == "--generations") {
            myAssert(hasValue, "Missing argument after " + key);
            arg_generations = atoi(val.c_str());
            i++;
            if (arg_generations < 1) {
                sayError("Parameter --gernerations must be > 0");
                return false;
            }
        }
        else if (key == "--mode") {
            arg_mode = val;
            myAssert(hasValue, "Missing argument after " + key);
            i++;
            if (arg_mode != "seq") {
                cout << "Error: invalid mode param\n";
                return false;
            }
        }
        else if (key == "--measure") {
            arg_measure = true;
        }
        else if (key == "--verbose") {
            arg_verbose = true;
        }
        else {
            sayWarning("Warning: unknown command line argument parameter '" + key + "'");
        }
    }
    
    if (arg_load == "") {
        sayError("Please specify a file to --load.");
        return false;
    }
    
    board = loadBoard(arg_load);
    
    sayVerbose("File successfully read (" + to_string(board.getColCount()) + "," + to_string(board.getRowCount()) + ").");
    
    if (arg_verify != "") {
        verificationBoard = loadBoard(arg_verify);
    }
    
    if (arg_generations < 1) {
        sayError("Please specify a --generations count!");
        return false;
    }
    
    return true;
}

Board Main::loadBoard(const std::string& path) {
    FILE *f = fopen(path.c_str(), "r");
    if(!f) {
        fail("fopen(r) failed for file at "+path);
    }
    int rowCount=-1, colCount=-1;
    fscanf(f, "%d,%d\n", &colCount, &rowCount);
    if(rowCount < 1 || colCount < 1) {
        fclose(f);
        fail("fscanf() failed for file at "+path);
    }
    char* buf = (char*)malloc(colCount+5);
    if(buf == nullptr) {
        fclose(f);
        fail("malloc failed for file at "+path);
    }
    Board board = Board(rowCount, colCount);
    for(int rowIdx=0; rowIdx<rowCount; rowIdx++) {
        char* result = fgets(buf, colCount+3, f);
        if(result == nullptr) {
            fclose(f);
            fail("fgets failed for file at "+path);
        }
        for(int colIdx=0; colIdx<colCount; colIdx++) {
            if(buf[colIdx] == 'x' || buf[colIdx] == 'X') {
                board.setCell(rowIdx, colIdx, true);
            }
        }
    }
    fclose(f);
    return board;
}

void Main::saveBoard(const Board& board, const std::string& path) {
    FILE* f = fopen(path.c_str(), "w");
    if(f == NULL) {
        fail("fopen(w) failed for file at "+path);
    }
    std::string s = board.toString();
    fwrite(s.c_str(), 1, s.length(), f);
    fclose(f);
}

void Main::logMessage(string msg) {
    high_resolution_clock::time_point now = high_resolution_clock::now();
    duration<double> diff = duration_cast<duration<double>>(now - timeStartInitPhase);
    string diffStr = formatDuration(diff);
    
    cout << diffStr << ": " << msg << endl;
}

string Main::formatDuration(double duration) {
    int seconds = (int)lround(floor(duration));
    int milliseconds = (int)lround((duration - seconds) * 1000);
    if (milliseconds > 999) {
        milliseconds = 999;
    }
    else if (milliseconds < 0) {
        milliseconds = 0;
    }
    
    int minutes = seconds / 60;
    int hours = minutes / 60;
    
    const int BUF_SIZE = 40;
    char buf[BUF_SIZE+1];
    snprintf(buf, BUF_SIZE, "%02d:%02d:%02d.%03d", hours, minutes % 60, seconds % 60, milliseconds);
    return string(buf);
}

int main(int argc, char **argv)
{
    Main m;
    int returnCode = m.run(argc, argv);
    return returnCode;
}

