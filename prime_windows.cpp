#include <iostream>
#include <windows.h>
#include <vector>
#include <string>
using namespace std;

bool estePrim(int n)
{
    if (n < 2) return false;
    for (int i = 2; i * i <= n; i++)
        if (n % i == 0) return false;
    return true;
}

int main(int argc, char* argv[])
{
    // ---------------- PROCES COPIL ----------------
    if (argc == 3 && string(argv[1]) == "child")   // dacă programul este lansat ca „child”
    {
        int start = atoi(argv[2]);                // intervalul primit
        int end   = start + 999;

        string rez = "";
        for (int i = start; i <= end; i++)        // calculează numere prime
            if (estePrim(i))
                rez += to_string(i) + " ";

        cout << rez;                               // trimite rezultatul în pipe
        return 0;
    }

    // ---------------- PROCES PĂRINTE ----------------
    cout << "Pornesc 10 procese pentru cautarea numerelor prime...\n";

    HANDLE hRead[10], hWrite[10];                 // pipe-uri pentru fiecare proces
    PROCESS_INFORMATION pi[10];                   // info procese copil
    STARTUPINFO si[10];                           // configurare STDOUT copil
    ZeroMemory(&pi, sizeof(pi));                  // inițializare

    for (int i = 0; i < 10; i++)
        ZeroMemory(&si[i], sizeof(si[i]));        // inițializare STARTUPINFO

    // --------- Creăm cele 10 procese + pipe-urile ---------
    for (int i = 0; i < 10; i++)
    {
        SECURITY_ATTRIBUTES sa;                   // permite moștenirea handle-urilor
        sa.nLength = sizeof(sa);
        sa.lpSecurityDescriptor = NULL;
        sa.bInheritHandle = TRUE;

        if (!CreatePipe(&hRead[i], &hWrite[i], &sa, 0))   // creăm pipe
        {
            cout << "Eroare la crearea pipe-ului!\n";
            return 1;
        }

        si[i].cb = sizeof(si[i]);
        si[i].hStdOutput = hWrite[i];             // copilul scrie în pipe
        si[i].hStdError  = hWrite[i];
        si[i].dwFlags = STARTF_USESTDHANDLES;

        int start = i * 1000 + 1;                 // intervalul procesului

        string comanda = string(argv[0]) +         // pornim copilul cu argumente
            " child " + to_string(start);

        if (!CreateProcessA(
            NULL,
            (LPSTR)comanda.c_str(),               // comanda lansată
            NULL, NULL,
            TRUE,                                 // copilul moștenește pipe-ul
            0, NULL, NULL,
            &si[i], &pi[i]))                      // info despre proces
        {
            cout << "Eroare la crearea procesului " << i + 1 << "\n";
        }

        CloseHandle(hWrite[i]);                   // părintele închide capătul de scriere
    }

    // --------- Citim rezultatele din pipe-uri ---------
    cout << "\nRezultatele primite de la procese:\n";

    char buffer[4096];
    DWORD bytesRead;

    for (int i = 0; i < 10; i++)
    {
        string rezultat = "";

        while (ReadFile(hRead[i], buffer, sizeof(buffer)-1, &bytesRead, NULL)
               && bytesRead > 0)                  // citim din pipe
        {
            buffer[bytesRead] = '\0';
            rezultat += buffer;
        }

        cout << "[Proces " << i + 1 << "] -> " << rezultat << "\n";

        CloseHandle(hRead[i]);                    // închidem pipe-ul
    }

    // --------- Așteptăm finalizarea proceselor copil ---------
    HANDLE handles[10];
    for (int i = 0; i < 10; i++)
        handles[i] = pi[i].hProcess;              // extragem handle-urile proceselor

    WaitForMultipleObjects(10, handles, TRUE, INFINITE);  // așteaptă toate procesele

    // --------- Închidere procese ---------
    for (int i = 0; i < 10; i++)
    {
        CloseHandle(pi[i].hProcess);
        CloseHandle(pi[i].hThread);
    }

    cout << "Toate procesele au terminat.\n";
    return 0;
}
