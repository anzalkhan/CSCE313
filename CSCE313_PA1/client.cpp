#include "common.h"
#include "FIFORequestChannel.h"
#include <sys/wait.h>
#include <fstream>
#include <chrono> // For timing
#include <algorithm> // For std::min

using namespace std;
using namespace std::chrono;

int main(int argc, char *argv[]) {
    int opt;
    int p = 1;
    double t = 0.0;
    int e = 1;
    string filename = "received/x1.csv";
    bool file_request = false;

    // Add other arguments here
    while ((opt = getopt(argc, argv, "p:t:e:f:")) != -1) {
        switch (opt) {
            case 'p':
                p = atoi(optarg);
                break;
            case 't':
                t = atof(optarg);
                break;
            case 'e':
                e = atoi(optarg);
                break;
            case 'f':
                filename = optarg;
                file_request = true;
                break;

        }
    }

    // Run the server process as a child of the client process
    pid_t childserver = fork();
    if (childserver < 0) {
        perror("Fork failed");
        exit(1);
    }

    if (childserver == 0) {
        char* args[] = {(char*) "./server", nullptr};
        execvp(args[0], args);
        perror("exec failed");
        exit(1);
    }

    // Create control channel
    FIFORequestChannel chan("control", FIFORequestChannel::CLIENT_SIDE);

	// Request a new channel from the server
    MESSAGE_TYPE new_channel_msg = NEWCHANNEL_MSG;
    chan.cwrite(&new_channel_msg, sizeof(MESSAGE_TYPE));  // Request a new channel

    // Read the name of the new channel
    char new_channel_name[30];
    chan.cread(new_channel_name, sizeof(new_channel_name));
    // cout << "New channel created: " << new_channel_name << endl;

    // Create a new channel instance using the received channel name
    FIFORequestChannel new_chan(new_channel_name, FIFORequestChannel::CLIENT_SIDE);

    if (file_request) {
        // Task 3: File transfer
        cout << "Requesting file: " << filename << endl;

        // Step 1: Send a filemsg to get the file length
        filemsg fm(0, 0);
        int len = sizeof(filemsg) + filename.size() + 1;
        char* file_buf = new char[len];  // Use separate buffer for file transfer
        memcpy(file_buf, &fm, sizeof(filemsg));
        strcpy(file_buf + sizeof(filemsg), filename.c_str());

        chan.cwrite(file_buf, len); // Request file length
        __int64_t file_length;
        chan.cread(&file_length, sizeof(__int64_t));
        cout << "File size: " << file_length << " bytes" << endl;

        // Step 2: Create output file in "received/" directory
        string output_file = "received/" + filename;
        ofstream outfile(output_file, ios::binary);
        if (!outfile.is_open()) {
            cerr << "Error opening output file: " << output_file << endl;
            exit(1);
        }

        // Step 3: Request file content in chunks
        const int chunk_size = MAX_MESSAGE - sizeof(filemsg);
        __int64_t offset = 0;

        auto start = chrono::high_resolution_clock::now();  // Start timing

        while (offset < file_length) {
            __int64_t bytes_to_read = min(static_cast<__int64_t>(chunk_size), file_length - offset); // Ensure types match
            filemsg fm_chunk(offset, bytes_to_read);
            memcpy(file_buf, &fm_chunk, sizeof(filemsg));
            strcpy(file_buf + sizeof(filemsg), filename.c_str());

            chan.cwrite(file_buf, len); // Request chunk of the file

            char* recv_buffer = new char[bytes_to_read];  // Dynamically allocate buffer
            chan.cread(recv_buffer, bytes_to_read);

            // Write to file
            outfile.write(recv_buffer, bytes_to_read);

            delete[] recv_buffer;  // Free memory after use

            // Move the offset forward
            offset += bytes_to_read;
        }

        outfile.close();

        auto end = chrono::high_resolution_clock::now();  // End timing
        auto duration = chrono::duration_cast<chrono::microseconds>(end - start).count();
        cout << "File transfer completed in " << duration << " microseconds." << endl;

        // Step 4: Compare files using 'diff' command
        string diff_cmd = "diff BIMDC/" + filename + " received/" + filename;
        int diff_result = system(diff_cmd.c_str());
        if (diff_result == 0) {
            cout << "Files are identical!" << endl;
        } else {
            cout << "Files differ!" << endl;
        }

        delete[] file_buf;  // Free file buffer

    } else {
        // Task 2: Request data points
        ofstream outfile(filename);
        if (!outfile.is_open()) {
            cerr << "Error opening file: " << filename << endl;
            exit(1);
        }

        for (int i = 0; i < 1000; ++i) {
            double time_point = i * 0.004;  

            // Request ECG1 data
            datamsg ecg1_msg(p, time_point, 1);
            chan.cwrite(&ecg1_msg, sizeof(datamsg));
            double ecg1_value;
            chan.cread(&ecg1_value, sizeof(double));

            // Request ECG2 data
            datamsg ecg2_msg(p, time_point, 2);
            chan.cwrite(&ecg2_msg, sizeof(datamsg));
            double ecg2_value;
            chan.cread(&ecg2_value, sizeof(double));

            // Write the time point, ECG1 value, and ECG2 value to the output file
            outfile << time_point << "," << ecg1_value << "," << ecg2_value << endl;
        }

        // Close the output file
        outfile.close();

        char buf[MAX_MESSAGE];
        datamsg x(p, t, e);
        memcpy(buf, &x, sizeof(datamsg));
        chan.cwrite(buf, sizeof(datamsg));
        double reply;
        chan.cread(&reply, sizeof(double));
        cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;
    }

    // Task 5: Closing the channel
    MESSAGE_TYPE m = QUIT_MSG;
    chan.cwrite(&m, sizeof(MESSAGE_TYPE));

    int check;
    waitpid(childserver, &check, 0);
    return 0;
}
