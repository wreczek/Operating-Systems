# Zadanie 1

Napisz program, który wykonuje wielowątkową operację filtrowania obrazu. Program przyjmuje w argumentach wywołania:

    * liczbę wątków,
    * sposób podziału obrazu pomiędzy wątki, t.j. jedną z dwóch opcji: block / interleaved,
    * nazwę pliku z wejściowym obrazem,
    * nazwę pliku z definicją filtru,
    * nazwę pliku wynikowego.

Po wczytaniu danych (wejściowy obraz i definicja filtru) wątek główny tworzy tyle nowych wątków, ile zażądano w argumencie wywołania. Utworzone wątki równolegle tworzą wyjściowy (filtrowany) obraz. Każdy stworzony wątek odpowiada za wygenerowanie części wyjściowego obrazu:

    Gdy program został uruchomiony z opcją block, k-ty wątek wylicza wartości pikseli w pionowym pasku o współrzędnych x-owych w przedziale od (k−1)∗ceil(N/m) do k∗ceil(N/m)−1, gdzie N to szerokość wyjściowego obrazu a m to liczba stworzonych wątków.
    Gdy program został uruchomiony z opcją interleaved, k-ty wątek wylicza wartości pikseli, których współrzędne x-owe to: k−1, k−1+m, k−1+2∗m, k−1+3∗m, itd. (ponownie, m to liczba stworzonych wątków).

Po wykonaniu obliczeń wątek kończy pracę i zwraca jako wynik (patrz pthread_exit) czas rzeczywisty spędzony na tworzeniu przydzielonej mu części wyjściowego obrazu. Czas ten należy zmierzyć z dokładnością do mikrosekund. Wątek główny czeka na zakończenie pracy przez wątki wykonujące operację filtrowania. Po zakończeniu każdego wątku, wątek główny odczytuje wynik jego działania i wypisuje na ekranie informację o czasie, jaki zakończony wątek poświęcił na filtrowanie obrazu (wraz z identyfikatorem zakończonego wątku). Dodatkowo, po zakończeniu pracy przez wszystkie stworzone wątki, wątek główny zapisuje powstały obraz do pliku wynikowego i wypisuje na ekranie czas rzeczywisty spędzony w całej operacji filtrowania (z dokładnością do mikrosekund). W czasie całkowitym operacji filtrowania należy uwzględnić narzut związany z utworzeniem i zakończeniem wątków (ale bez czasu operacji wejścia/wyjścia).
Wykonaj pomiary czasu operacji filtrowania dla obrazu o rozmiarze kilkaset na kilkaset pikseli i dla kilku filtrów (można wykorzystać losowe macierze filtrów). Testy przeprowadź dla 1, 2, 4, i 8 wątków. Rozmiar filtrów dobierz w zakresie 3≤c≤65
, tak aby uwidocznić wpływ liczby wątków na czas operacji filtrowania. Eksperymenty wykonaj dla obu wariantów podziału obrazu pomiędzy wątki (block  i interleaved). Wyniki zamieść w pliku Times.txt i dołącz do archiwum z rozwiązaniem zadania.

### Format wejścia-wyjścia

Program powinien odczytywać i zapisywać obrazy w formacie ASCII PGM (Portable Gray Map). Pliki w tym formacie mają nagłówek postaci:

P2
W H
M
...

gdzie: W to szerokość obrazu w pikselach, H to wysokość obrazu w pikselach a M to maksymalna wartość piksela. Zakładamy, że obsługujemy jedynie obrazy w 256 odcieniach szarości: od 0 do 255 (a więc M=255). Po nagłówku, w pliku powinno być zapisanych W*H liczb całkowitych reprezentujących wartości kolejnych pikseli. Liczby rozdzielone są białymi znakami (np. spacją). Piksele odczytywane są wierszami, w kolejności od lewego górnego do prawego dolnego rogu obrazu.

Przykładowe obrazy w formacie ASCII PGM (jak również opis formatu) można znaleźć pod adresem:
http://people.sc.fsu.edu/~jburkardt/data/pgma/pgma.html

W pierwszej linii pliku z definicją filtru powinna znajdować się liczba całkowita c
określająca rozmiar filtru. Dalej, plik powinien zawierać c2 liczb zmiennoprzecinkowych określających wartości elementów filtru (w kolejności wierszy, od elementu K[1,1] do elementu K[c,c]).
