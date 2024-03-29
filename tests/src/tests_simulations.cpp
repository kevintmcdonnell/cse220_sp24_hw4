#include "test_utils.h"
#include <fstream>

extern "C" {
#include "hw4.h"
}

class simulations_TestSuite : public testing::Test {
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

bool run_simulation01() {
    INFO("Testing student server with client solution.");
    system("rm -f ./fen.txt");  // File to be produced before client solution terminates.
    system("rm -f pipe.server pipe.client");
    system("mkfifo pipe.server pipe.client");

    pthread_t sleep_tid1, sleep_tid2;
    pthread_create(&sleep_tid1, NULL, system_thread, (void *)"sleep infinity > pipe.server &");
    pthread_join(sleep_tid1, NULL);
    pthread_create(&sleep_tid2, NULL, system_thread, (void *)"sleep infinity > pipe.client &");
    pthread_join(sleep_tid2, NULL);

    int server_pid, client_pid;
    int ret = system("netstat -an | fgrep '0.0.0.0:8080' > /dev/null");
    if (WEXITSTATUS(ret) == 0) {
        std::cerr << "Server was already running" << std::endl;
        return false;
    }

    if ((server_pid = fork()) == 0) {
        FILE *server_input = freopen("pipe.server", "r", stdin);
        if (server_input == nullptr) {
            perror("failed to redirect pipe.server to standard input\n");
            abort();
        }
        FILE *server_stderr = freopen("server_stderr.log", "w", stderr);
        if (server_stderr == nullptr) {
            perror("failed to redirect server stderr");
            abort();
        }
        INFO("Attempting to start server...");
        execl("./build/hw4_server", "hw4_server", NULL);
        fprintf(stderr, "Failed to exec server\n");
        abort();
    }

    sleep(1);
    if ((client_pid = fork()) == 0) {
        FILE *client_input = freopen("pipe.client", "r", stdin);
        if (client_input == nullptr) {
            perror("failed to redirect pipe.client to standard input\n");
            abort();
        }
        FILE *client_stderr = freopen("client_stderr.log", "w", stderr);
        if (client_stderr == nullptr) {
            perror("failed to redirect client stderr");
            abort();
        }
        INFO("Attempting to start client...");
        execl("./build/hw4_client_soln", "hw4_client_soln", NULL);
        fprintf(stderr, "Failed to exec client\n");
        abort();
    }

    system("echo /move e2e4 > pipe.client");
    sleep(1);
    system("echo /move e7e5 > pipe.server");
    sleep(1);
    system("echo /forfeit > pipe.client");
    sleep(1);

    kill(server_pid, SIGKILL);
    kill(client_pid, SIGKILL);
    sleep(1);

    int status;
    waitpid(client_pid, &status, WNOHANG);
    EXPECT_EQ(1, WIFEXITED(status)) << "Client process did not terminate." << std::endl;
    waitpid(server_pid, &status, WNOHANG);
    EXPECT_EQ(1, WIFEXITED(status)) << "Server process did not terminate." << std::endl;    

    kill(sleep_tid1, SIGKILL);
    kill(sleep_tid2, SIGKILL);
    sleep(1);

    system("rm -f pipe.server pipe.client");

    char *actual_fen = load_file("./fen.txt");
    const char *expected_fen = "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w";
    if (actual_fen == nullptr) {
        std::cerr << "Client did not produce output." << std::endl;
        return false;
    }
    if (strcmp(actual_fen, expected_fen) != 0) {
        std::cerr << "Client output does not match expected output." << std::endl;
        return false;
    }

    return true;
}

void *simulation01_thread(void *arg) {
    (void)arg;
    bool result = run_simulation01();
    return (void *)result;
} 


TEST_F(simulations_TestSuite, simulation01_hot_garbage_test) {
    ASSERT_TRUE(run_simulation01());
    // write to a file that the simulation passed
    std::ofstream file("./tests/actual_outputs/simulation01_passed.txt");
    file << "simulation01_passed is true." << std::endl;
    file.close();
}
