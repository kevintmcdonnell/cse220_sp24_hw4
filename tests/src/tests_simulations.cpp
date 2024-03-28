#include "test_utils.h"
#include <fstream>

extern "C" {
#include "hw4.h"
}

class hw4_TestSuite : public testing::Test { 
protected:
    void SetUp() override {
        mkdir("./tests/expected_outputs/", 0700);
        mkdir("./tests/actual_outputs/", 0700);
    }
};


void *system_thread(void *arg) {
    long ret = system((char *)arg);
    return (void *)ret;
}

TEST_F(hw4_TestSuite, simulation01) {
    INFO("Testing student server with client solution.");
    system("rm -f ./fen.txt"); // File to be produced before client solution terminates.
    system("rm -f pipe.server pipe.client");
    system("mkfifo pipe.server pipe.client");
    pthread_t sleep_tid1, sleep_tid2;
    pthread_create(&sleep_tid1, NULL, system_thread, (void *)"sleep infinity > pipe.server &");  
    pthread_join(sleep_tid1, NULL); 
    pthread_create(&sleep_tid2, NULL, system_thread, (void *)"sleep infinity > pipe.client &");  
    pthread_join(sleep_tid2, NULL); 

    int server_pid, client_pid;
    int ret = system("netstat -an | fgrep '0.0.0.0:8080' > /dev/null");
    ASSERT_NE(0, WEXITSTATUS(ret)) << "Server was already running" << std::endl;
   
    if((server_pid = fork()) == 0) {
        int fd = open("pipe.server", O_RDONLY);
        if (fd < 0)
            perror("failed to open file pipe.server for reading\n");
        if (dup2(fd, STDIN_FILENO) < 0)
            perror("failed to redirect pipe.server to standard input\n");
        close(fd);  

        INFO("Attempting to start server...");
        execl("./build/hw4_server", "hw4_server", NULL);
        fprintf(stderr, "Failed to exec server\n");
        abort();
    }
    sleep(1);
    if((client_pid = fork()) == 0) {
        int fd = open("pipe.client", O_RDONLY);
        if (fd < 0)
            perror("failed to open file pipe.client for reading\n");
        if (dup2(fd, STDIN_FILENO) < 0)
            perror("failed to redirect pipe.client to standard input\n");
        close(fd);  

        INFO("Attempting to start client...");
        execl("./build/hw4_client_soln", "hw4_client_soln", NULL);
        fprintf(stderr, "Failed to exec client\n");
        abort();
    }
    // Begin: send commands to client and server
    system("echo /move e2e4 > pipe.client");
    sleep(1);
    system("echo /move e7e5 > pipe.server");
    sleep(1);
    system("echo /forfeit > pipe.client");
    sleep(1);
    // End: commands sent to client and server

    kill(server_pid, SIGKILL);
    kill(client_pid, SIGKILL);
    sleep(1);

    int status;
    waitpid(client_pid, &status, WNOHANG);
    EXPECT_EQ(1, WIFEXITED(status)) << "Client process did not terminate." << std::endl;

    kill(sleep_tid1, SIGKILL);
    kill(sleep_tid2, SIGKILL);
    sleep(1);
    system("rm -f pipe.server pipe.client");
    
    char *actual_fen = load_file("./fen.txt");
    const char *expected_fen = "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w";
    ASSERT_NE(nullptr, actual_fen) << "Client did not produce output." << std::endl;
    ASSERT_STREQ(actual_fen, expected_fen) << "Client output does not match expected output." << std::endl;
}

