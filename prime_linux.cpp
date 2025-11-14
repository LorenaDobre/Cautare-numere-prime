#include <iostream>
#include <unistd.h>     // pentru fork(), pipe(), read(), write()
#include <sys/wait.h>   // pentru wait()
#include <vector>
using namespace std;

bool estePrim(int n) {
    if (n < 2) return false;             // 0 și 1 nu sunt prime
    for (int i = 2; i * i <= n; i++)     // verificăm până la rădăcina pătrată a lui n
        if (n % i == 0)                  // dacă găsim un divizor, nu e prim
            return false;
    return true;                         // altfel, numărul este prim
}

int main() {
    const int nrProcese = 10;            // vom crea 10 procese copil
    int pipe_fd[nrProcese][2];           // vector 2D cu 10 pipe-uri (câte unul pentru fiecare copil)

    // Crearea celor 10 pipe-uri
    for (int i = 0; i < nrProcese; i++) {
        if (pipe(pipe_fd[i]) == -1) {            // se creează un nou pipe
            perror("Eroare la crearea pipe-ului"); // mesaj în caz de eroare
            return 1;
        }
    }
    // Crearea celor 10 procese copil
    for (int i = 0; i < nrProcese; i++) {
        pid_t pid = fork();                      // se creează un proces copil
        if (pid < 0) {                           // verific dacă fork() a eșuat
            perror("Eroare la fork");
            return 1;
        }
        if (pid == 0) {                          // dacă pid == 0 => suntem în procesul copil
            close(pipe_fd[i][0]);                // copilul nu citește, deci închide capătul de citire
            int start = i * 1000 + 1;            // primul număr din intervalul procesului
            int end = (i + 1) * 1000;            // ultimul număr din interval

            vector<int> primes;                  // vector în care salvez numerele prime

            // Căutarea numerelor prime din intervalul alocat
            for (int j = start; j <= end; j++) { // parcurg fiecare număr din interval
                if (estePrim(j))                 // verific dacă este prim
                    primes.push_back(j);         // dacă da, îl adaug în vector
            }
            // Trimiterea numerelor prime către procesul părinte
            for (int p : primes) {               // parcurg fiecare număr prim găsit
                write(pipe_fd[i][1], &p, sizeof(p)); // trimit numărul prin pipe
            }

            int stop = -1;                       // valoare specială care indică finalul datelor
            write(pipe_fd[i][1], &stop, sizeof(stop)); // trimit semnalul de final
            close(pipe_fd[i][1]);                // închid capătul de scriere
            _exit(0);                            // termin execuția copilului
        }
    }
    // Procesul părinte citește datele trimise de fiecare copil
    for (int i = 0; i < nrProcese; i++) {
        close(pipe_fd[i][1]);                    // părintele nu scrie, deci închide capătul de scriere

        int numar;                               // variabilă pentru valorile primite
        cout << "Proc " << i + 1 << " → ";       // afișez prefixul pentru procesul respectiv

    // Citirea numerelor primite prin pipe
        while (read(pipe_fd[i][0], &numar, sizeof(numar)) > 0 && numar != -1) {
            cout << numar << " ";                // afișez fiecare număr prim primit
        }

        cout << endl;                            // trec pe o linie nouă după fiecare proces
        close(pipe_fd[i][0]);                    // închid capătul de citire
    }
    // Aștept finalizarea tuturor proceselor copil
    for (int i = 0; i < nrProcese; i++) {
        wait(NULL);                              // părintelui așteaptă terminarea fiecărui copil
    }
    return 0;
}
