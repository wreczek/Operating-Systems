
## Opis problemu
Jednym z często spotykanych problemów synchronizacji jest "Problem producentów i konsumentów". Grupa producentów zapisuje dane w określonym miejscu, a konsumenci je pobierają (konsumują). Jedno z rozwiązań opiera się na wykorzystaniu tablicy działającej jak bufor cykliczny, co pozwala na zamortyzowanie chwilowych różnic w szybkości działania producenta i konsumenta oraz niedopuszczenie do nadprodukcji. Tę wersję problemu nazywa się problemem z ograniczonym buforem. Pojedynczy element bufora jest sekcją krytyczną, w której przebywać może tylko jeden producent albo konsument.
  
Zaimplementuj program P+K wątkowy dla P producentów i K konsumentów, działających na tym samym buforze. Rolę bufora pełni globalna tablica N wskaźników do stringów o różnej długości. Producent produkuje i wstawia do bufora porcje różnej wielkości. Konsument pobiera, poszukując porcji określonej wielkości.

Producent działa następująco:

    * Jeżeli bufor nie jest pełny, to producent czyta kolejną linię z pliku tekstowego zadanego jako parametr programu (powinien mieć kilka tysięcy linii, np całość "Pana Tadeusza"), alokuje pamięć i umieszcza w buforze wskaźnik do tekstu.
    * Jeżeli bufor jest pełny, to producent powinien zaczekać do momentu usunięcia wartości z bufora przez konsumenta.
    * Każda kolejna wartość jest produkowana w następnym elemencie bufora (producent nie zaczyna od początku tablicy, szukając pierwszego wolnego miejsca, lecz pamięta pozycję, gdzie poprzedni producent wstawił wartość, co zapewnia równomierną produkcję dla całego bufora.
    * Po wstawieniu wartości do ostatniego elementu tablicy producent zaczyna cyklicznie wstawiać elementy  od początku tablicy.

Konsument działa następująco:

    * jeżeli bufor jest pusty, to konsument powinien zaczekać do momentu umieszczenia wartości w buforze przez producenta.
    * jeżeli bufor nie jest pusty, to konsument pobiera, usuwa wartość z bufora i sprawdza, czy długość pobranego napisu jest w zależności od wartości podanego argumentu równa, większa, bądź mniejsza podanej jako argument wartości L, jeśli tak, to wypisuje nr indeksu tablicy i ten napis.
    * praca konsumentów jest analogiczna do pracy producentów (konsument nie zaczyna od początku, szukając pierwszego wyprodukowanego elementu, ale pamięta pozycję, gdzie poprzedni konsument pobrał wartość, co zapewnia równomierną konsumpcję dla całego bufora.
    * Po pobraniu wartości ostatniego elementu tablicy konsument zaczyna cyklicznie pobierać elementy  od początku tablicy.

Wątki powinny działać w pętli nieskończonej i kończyć się:

    * jeśli nk>0, po upływie nk sekund,
    * jeśli nk=0, po przeczytaniu ostatniego wersu pliku tekstowego lub po odebraniu przez proces główny sygnału CTRL-C.

Program powinien umożliwić uruchomienie trybu opisowego (każdy producent i konsument raportuje swoją pracę) oraz uproszczonego (informacje wypisują tylko konsumenci, jeśli odnajdą odpowiedni wynik).

Program powinien wczytać z linii poleceń plik konfiguracyjny, w którym są ustawione parametry P, K, N, nazwa pliku, L, tryb wyszukiwania, tryb wypisywania informacji oraz nk.
Należy wykonać dwie wersje rozwiązania:

    1. Rozwiązanie wykorzystujące do synchronizacji muteks i zmienne warunkowe (zgodne z typem rozwiązań problemu współbieżnego stosującego monitor) (50%)
    2. Rozwiązanie wykorzystujące do synchronizacji semafory nienazwane standardu POSIX (zgodne z typem rozwiązań problemu współbieżnego stosującego semafory) (50%)
