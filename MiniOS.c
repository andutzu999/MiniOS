/* BLANARU ANDY-STEFAN
GRUPA: 312CB */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct Celula {
	struct celula* urm;
	void* info;
}celula;

typedef struct celulaStiva {
	struct celStiva* urm;
	void* info;
}celStiva;

typedef struct Stiva {
	size_t dim;
	celStiva* top;
}Stack;

typedef struct Proces {
	int mem_size;
	int mem_start;
	int PID;
	int prioritate;
	int timp_executie;
	int cuantum_timp;
	int timp_executat;
	int stack_size;
	Stack* stiva;
}proces;

typedef struct Coada {
	celula* front, * rear;
	size_t dim;
}Queue;

typedef struct Memorie {
	void* info;
	struct Memorie* urm;
}Memorie;

int vidaQ(void* Q)
{
	if (((Queue*)Q)->front == NULL && ((Queue*)Q)->rear == NULL) return 1;
	return 0;
}

int vidaS(void* s)
{
	if (((Stack*)s)->top == NULL) return 1;
	return 0;
}

void* initQ(size_t dim) {
	Queue* Q;
	Q = (Queue*)calloc(1, dim);
	Q->dim = dim;
	return (void*)Q;
}

void* initS(size_t dim) {
	Stack* S;
	S = (Stack*)calloc(1, dim);
	S->dim = dim;
	return (void*)S;
}

proces* alocareProces() {
	proces* p;
	p = (proces*)calloc(1, sizeof(proces));
	if (p == NULL) {
		printf("Nu s-a putut aloca procesul!\n");
		return NULL;
	}
	p->stiva = (Stack*)calloc(1, sizeof(Stack));
	if (!p->stiva) return NULL;
	p->stiva->dim = sizeof(Stack);
	p->stiva->top = NULL;
	return p;
}

celStiva* alocareCelStiva(void* p, size_t dim) {
	celStiva* aux = (celStiva*)calloc(1, sizeof(celStiva));
	if (aux == NULL) {
		printf("Nu s-a putut aloca celula!\n");
		return NULL;
	}
	aux->urm = NULL;
	aux->info = malloc(dim);
	if (aux->info == NULL) {
		printf("Nu s-a putut aloca informatia!\n");
		return NULL;
	}
	memcpy(aux->info, &p, dim);
	return aux;
}

celula* alocareCelula(void* p, size_t dim) {
	celula* aux;
	aux = (celula*)calloc(1, sizeof(celula));
	if (aux == NULL) {
		printf ("Nu s-a putut aloca celula!\n");
		return;
	}
	aux->urm = NULL;
	aux->info = malloc(dim);
	if (aux->info == NULL) {
		printf("Nu s-a putut aloca informatia!\n");
		return;
	}
	memcpy(aux->info, p, dim);
	return aux;
}

celula* extrQ(void* Q) {
	if (((Queue*)Q)->front == ((Queue*)Q)->rear)
	{
		celula* aux = ((Queue*)Q)->front;
		((Queue*)Q)->front = NULL;
		((Queue*)Q)->rear = NULL;
		return aux;
	}
	celula* aux = ((Queue*)Q)->front;
	((Queue*)Q)->front = aux->urm;
	return aux;
}

celStiva* popS(Stack* s)
{
	if (s->top->urm == NULL)
	{
		celStiva* aux = s->top;
		s->top = NULL;
		return aux;

	}
	celStiva* aux = s->top;
	s->top = aux->urm;
	return aux;

}

celula* topQ(void* Q) {
	return ((Queue*)Q)->front;
}

void intrQ(Queue * *Q, void* p, size_t d)
{
	celula* aux;
	aux = alocareCelula(p, d);
	if (aux == NULL) {
		printf("Nu s-a putut aloca celula!\n");
		return;
	}
	if ((*Q)->front == NULL && (*Q)->rear == NULL)
	{
		(*Q)->front = aux;
		(*Q)->rear = aux;
	}
	else
	{
		(*Q)->rear->urm = aux;
		(*Q)->rear = aux;
	}
}

void* push(Stack* s, void *info)
{
	celStiva* aux = (celStiva*)calloc(1, sizeof(celStiva));
	aux->info = info;
	if (!aux) return NULL;
	if (vidaS(s) == 1)
	{
		s->top = aux;
		return (void*)s;
	}
	else
	{
		aux->urm = s->top;
		s->top = aux;
	}
	return (void*)s;
}

// Adaug datele in structura de tip proces
proces* setProcesDate(proces * proces_nou, int PID, int mem_size, int exec_time, int prioritate, int cuantum_timp) {
	proces_nou->mem_size = mem_size;
	proces_nou->timp_executie = exec_time;
	proces_nou->prioritate = prioritate;
	proces_nou->PID = PID;
	proces_nou->timp_executat = 0;
	proces_nou->cuantum_timp = cuantum_timp;
	proces_nou->stack_size = 0;
	return proces_nou;
}

// Calculez si returnez cel mai mic PID disponibil
int determinarePID(int** vect_PID) {
	int i = 1;
	for (i; i < 32768; i++)
		if ((*vect_PID)[i] == 0) {
			(*vect_PID)[i] = 1;
			return i;
		}
	return -1;
}

void adaugaMemorie(Memorie **memorie, proces **proces_nou, int *vect_PID) {
	// Daca nu am memorie, atunci o aloc. Daca nu am niciun element atunci il inserez la inceput.
	if ((*memorie) == NULL) (*memorie) = (Memorie*)calloc(1, sizeof(Memorie));
	if ((*memorie)->info == NULL)
		(*memorie)->info = (*proces_nou);
	// Daca nu am al doilea element, atunci il inserez la sfarsit.
	else if ((*memorie)->urm == NULL) {
		(*proces_nou)->mem_start = ((proces*)(*memorie)->info)->mem_size;
		(*memorie)->urm = (Memorie*)calloc(1, sizeof(Memorie));
		(*memorie)->urm->info = (*proces_nou);
	}
	else {
		Memorie* aux = (*memorie);
		// Daca primul block de memorie nu are memoria de start 0, atunci verific
		// daca pot insera un element la inceput
		if (((proces*)aux->info)->mem_start != 0) {
			if ((*proces_nou)->mem_size <= ((proces*)aux->info)->mem_start) {
				Memorie* swap = (Memorie*)calloc(1, sizeof(Memorie));
				swap->info = (*proces_nou);
				swap->urm = (*memorie);
				(*memorie) = swap;
				return;
			}
		}
		// Daca nu pot insera la inceput, atunci parcurg block-urile de memorie, si verific daca intre doua block-uri
		// pot insera block-ul dat ca parametru in aceasta functie.
		while (aux->urm != NULL) {
			// Aceasta formula calculeaza marimea memoriei dintre doua block-uri consecutive, si verific daca block-ul
			// proces_nou intra intre cele doua block-uri
			if ((*proces_nou)->mem_size <= (((proces*)aux->urm->info)->mem_start - (((proces*)aux->info)->mem_size + ((proces*)aux->info)->mem_start))) {
				Memorie* swap = (Memorie*)calloc(1, sizeof(Memorie));
				swap->info = (*proces_nou);
				((proces*)swap->info)->mem_start = ((proces*)aux->info)->mem_start + ((proces*)aux->info)->mem_size;
				(*proces_nou)->mem_start = ((proces*)swap->info)->mem_start;
				swap->urm = aux->urm;
				aux->urm = swap;
				return;
			}
			aux = aux->urm;
		}
		// Daca am ajuns la sfarsitul block-urilor de memorie, inseamna ca nu am gasit loc pentru acel block, si il vom insera la sfarsit
		if (aux->urm == NULL) {
			Memorie* mem_nou = (Memorie*)calloc(1, sizeof(Memorie));
			(*proces_nou)->mem_start = ((proces*)aux->info)->mem_size + ((proces*)aux->info)->mem_start;
			mem_nou->info = (*proces_nou);
			aux->urm = mem_nou;
		}
	}
}

int concatQ(void* left, void* right)
{
	if (vidaQ(left) == 1)
	{
		((Queue*)left)->front = ((Queue*)right)->front;
		((Queue*)left)->rear = ((Queue*)right)->rear;
		return 1;
	}
	else
	{
		((Queue*)left)->rear->urm = ((Queue*)right)->front;
		((Queue*)left)->rear = ((Queue*)right)->rear;
		((Queue*)right)->rear = NULL;
		((Queue*)right)->front = NULL;
	}
	return 1;
}

int concatS(void* left, void* right)
{
	if (vidaS(left) == 1)
		((Stack*)left)->top = ((Stack*)right)->top;
	return 1;
}

int nrElemQ(Queue* coada) {
	int nr = 0;
	Queue* coada_aux = initQ(sizeof(Queue));
	celula* aux = coada->front;
	while (vidaQ(coada) == 0) {
		nr++;
		aux = extrQ(coada);
		intrQ(&coada_aux, (proces*)aux->info, sizeof(proces));
	}
	concatQ(coada, coada_aux);
	return nr;
}

void print_waiting(Queue* coada_asteptare, FILE* output) {
	celula* aux = coada_asteptare->front;
	Queue* coada_aux = initQ(sizeof(Queue));
	int nr = nrElemQ(coada_asteptare);
	fprintf(output, "Waiting queue:\n[");
	if (nr == 0){
		fprintf(output, "]\n"); 
		return;
	}
	if (coada_asteptare->front != NULL) {
		while (vidaQ(coada_asteptare) == 0) {
			aux = extrQ(coada_asteptare);
			nr--;
			intrQ(&coada_aux, (proces*)aux->info, sizeof(proces));
			fprintf(output, "(%d: priority = %d, remaining_time = %d)", ((proces*)aux->info)->PID, ((proces*)aux->info)->prioritate, ((proces*)aux->info)->timp_executie);
			if (nr == 0) fprintf(output, "]\n");
			else fprintf(output, ",\n");
		}
	}
	concatQ(coada_asteptare, coada_aux);
}

// Functia care imi introduce elemente in coada de asteptare si mentine ordinea precizata in enunt
void introducereQ(Queue** Q, proces* pr, FILE* output) {
	celula* aux = (*Q)->front;
	proces* proces_nou = pr;
	celula* cel = alocareCelula(proces_nou, sizeof(proces));
	Queue* coada_noua = initQ(sizeof(Queue));
	int prioritate = proces_nou->prioritate, exec_time = proces_nou->timp_executie;
	int PID = proces_nou->PID, introdus = 0, aux_null = 0;

	// Daca coada este nula, inseram elementul
	if ((*Q)->front == NULL) {
		intrQ(&coada_noua, proces_nou, sizeof(proces));
		aux_null = 1;
	}
	else {
		// Verific elementul pe care trebuie sa il introduc, cu fiecare element din coada.
		// Daca nu i-am gasit pozitia, introduc fiecare element din coada, intr-o coada noua.
		// Cand ii gasesc pozitia, introduc si elementul meu in coada, deci coada ramane sortata.
		// La final parcurg restul cozii, si introduc elementele ramase in coada, si reactualizez coada.
		while (vidaQ((*Q)) == 0) {
			aux = extrQ(*Q);
			if (prioritate > ((proces*)aux->info)->prioritate) {
				introdus = 1;
				intrQ(&coada_noua, proces_nou, sizeof(proces));
			}
			else if (prioritate == ((proces*)aux->info)->prioritate) {
				// Crescator dupa timp_executie
				if (exec_time < ((proces*)aux->info)->timp_executie) {
					introdus = 1;
					intrQ(&coada_noua, proces_nou, sizeof(proces));
				}
				else if (exec_time == ((proces*)aux->info)->timp_executie) {
					// Crescator dupa PID
					if (PID < ((proces*)aux->info)->PID) {
						introdus = 1;
						intrQ(&coada_noua, proces_nou, sizeof(proces));
					}
					else {
						intrQ(&coada_noua, (proces*)aux->info, sizeof(proces));
						continue;
					}
				}
				else {
					intrQ(&coada_noua, (proces*)aux->info, sizeof(proces));
					continue;
				}
			}
			else intrQ(&coada_noua, (proces*)aux->info, sizeof(proces));
			if (introdus == 1) break;
		}

		// Adaug elementele ramase
		if (aux != NULL) {
			intrQ(&coada_noua, (proces*)aux->info, sizeof(proces));
		}
		while (vidaQ((*Q)) == 0) {
			aux = extrQ((*Q));
			intrQ(&coada_noua, (proces*)aux->info, sizeof(proces));
		}
	}
	concatQ((*Q), coada_noua);
}

void add(Queue **Q, Memorie **memorie, int *nr_elemente, int *memorie_totala, int mem_size, int exec_time, int prioritate, int cuantum_timp, int** vect_PID, int* stare_running, celula * *running, FILE* output) {
	// Functia de add in coada. In primul rand verific daca adaugarea noului element intrece limita de 3MiB
	if ((*memorie_totala) + mem_size <= 3145728) {
		// Actualizez memoria totala si verific daca am element in starea de running
		(*memorie_totala) += mem_size;
		if (*stare_running == 0 || (*running) == NULL) {
			// Daca nu am, il adaug in memorie 
			if ((*running) == NULL) (*Q) = (Queue*)calloc(1, sizeof(Queue));
			(*stare_running) = 1;
			int PID = determinarePID(vect_PID);
			proces* proces_nou = alocareProces();
			proces_nou = setProcesDate(proces_nou, PID, mem_size, exec_time, prioritate, cuantum_timp);
			proces_nou->mem_start = 0;
			(*running) = alocareCelula(proces_nou, sizeof(proces));
			adaugaMemorie(memorie, &proces_nou, (*vect_PID));
			(*nr_elemente) = 1;
			fprintf(output, "Process created successfully: PID: %d, Memory starts at 0x%x.\n", proces_nou->PID, proces_nou->mem_start);
			return;
		}
		// Daca am element in starea running, atunci aplic acelasi algoritm ca la introducereQ
		int PID = determinarePID(vect_PID);
		proces* proces_nou = alocareProces();
		proces_nou = setProcesDate(proces_nou, PID, mem_size, exec_time, prioritate, cuantum_timp);
		adaugaMemorie(memorie, &proces_nou, (*vect_PID));
		(*nr_elemente)++;
		Queue* coada_noua = initQ(sizeof(Queue));
		celula* aux = (*Q)->front;
		int introdus = 0, aux_null = 0;
		if ((*Q)->front == NULL) {
			intrQ(&coada_noua, proces_nou, sizeof(proces));
			aux_null = 1;
		}
		else {
			// Verific elementul pe care trebuie sa il introduc, cu fiecare element din coada.
			// Daca nu i-am gasit pozitia, introduc fiecare element din coada, intr-o coada noua.
			// Cand ii gasesc pozitia, introduc si elementul meu in coada, deci coada ramane sortata
			// La final parcurg restul cozii, si introduc elementele ramase in coada, si reactualiez coada
			while (vidaQ((*Q)) == 0) {
				aux = extrQ(*Q);
				if (prioritate > ((proces*)aux->info)->prioritate) {
					introdus = 1;
					intrQ(&coada_noua, proces_nou, sizeof(proces));
				}
				else if (prioritate == ((proces*)aux->info)->prioritate) {
					// Crescator dupa timp_executie
					if (exec_time < ((proces*)aux->info)->timp_executie) {
						introdus = 1;
						intrQ(&coada_noua, proces_nou, sizeof(proces));
					}
					else if (exec_time == ((proces*)aux->info)->timp_executie) {
						// Crescator dupa PID
						if (PID < ((proces*)aux->info)->PID) {
							introdus = 1;
							intrQ(&coada_noua, proces_nou, sizeof(proces));
						}
						else {
							intrQ(&coada_noua, (proces*)aux->info, sizeof(proces));
							continue;
						}
					}
					else {
						intrQ(&coada_noua, (proces*)aux->info, sizeof(proces));
						continue;
					}
				}
				else intrQ(&coada_noua, (proces*)aux->info, sizeof(proces));
				if (introdus == 1) break;
			}
			// Adaug elementele ramase
			if (aux != NULL && introdus == 1) {
				intrQ(&coada_noua, (proces*)aux->info, sizeof(proces));
			}
			while (vidaQ((*Q)) == 0) {
				aux = extrQ((*Q));
				intrQ(&coada_noua, (proces*)aux->info, sizeof(proces));
			}
		}
		// Un caz mai ciudat in care nu gaseste locul in care sa introduca elementul, asa ca il introduc la sfarsit
		if (introdus == 0 && aux_null == 0) intrQ(&coada_noua, proces_nou, sizeof(proces));
		fprintf(output, "Process created successfully: PID: %d, Memory starts at 0x%x.\n", proces_nou->PID, proces_nou->mem_start);
		concatQ((*Q), coada_noua);
		return;
	}
	else {
		// Nu s-a putut adauga elementul din cauza lipsei de spatiu
		int PID = determinarePID(vect_PID);
		fprintf(output, "Cannot reserve memory for PID %d.\n", PID);
		(*vect_PID)[PID] = 0;
		return;
	}
}

void get(int PID, Queue * coada_asteptare, celula * running, Queue * finished, FILE* output) {
	// Parcurg starea running, coada de asteptare si coada finished, si afisez starea PID-ului
	Queue* coada_aux_1 = initQ(sizeof(Queue));
	Queue* coada_aux_2 = initQ(sizeof(Queue));
	int afisat = 0;
	if (running!=NULL)
		if (PID == ((proces*)running->info)->PID) {
			fprintf(output, "Process %d is running (remaining_time: %d).\n", PID, ((proces*)running->info)->timp_executie);
			return;
		}

	celula* aux = coada_asteptare->front;
	while (vidaQ(coada_asteptare) == 0) {
		aux = extrQ(coada_asteptare);
		intrQ(&coada_aux_1, (proces*)aux->info, sizeof(proces));
		if (PID == ((proces*)aux->info)->PID) {
			fprintf(output,  "Process %d is waiting (remaining_time: %d).\n", PID, ((proces*)aux->info)->timp_executie);
			afisat = 1;
		}
	}
	concatQ(coada_asteptare, coada_aux_1);

	aux = finished->front;
	while (vidaQ(finished) == 0) {
		aux = extrQ(finished);
		intrQ(&coada_aux_2, (proces*)aux->info, sizeof(proces));
		if (PID == ((proces*)aux->info)->PID) {
			fprintf(output, "Process %d is finished.\n", PID);
			afisat = 1;
		}
	}
	concatQ(finished, coada_aux_2);
	if(afisat == 0) fprintf(output,  "Process %d not found.\n", PID);
}

void print_finished(Queue * coada_finished, FILE* output) {
	celula* aux = coada_finished->front;
	Queue* coada_aux = initQ(sizeof(Queue));
	int nr = nrElemQ(coada_finished);
	fprintf(output, "Finished queue:\n[");
	if (nr == 0) {
		fprintf(output, "]\n");
		return;
	}
	if (coada_finished->front != NULL) {
		while (vidaQ(coada_finished) == 0) {
			aux = extrQ(coada_finished);
			nr--;
			intrQ(&coada_aux, (proces*)aux->info, sizeof(proces));
			fprintf(output, "(%d: priority = %d, executed_time = %d)", ((proces*)aux->info)->PID, ((proces*)aux->info)->prioritate, ((proces*)aux->info)->timp_executat);
			if (nr == 0) fprintf(output, "]\n");
			else fprintf(output, ",\n");
		}
	}
	concatQ(coada_finished, coada_aux);
}

void remove_memorie(int PID, Queue** coada_asteptare, Memorie** memorie, int** vect_PID, int* nr_elemente) {
	Memorie* aux = (*memorie);
	(*vect_PID)[PID] = 0;
	(*nr_elemente)--;
	// Daca numarul de elemente este 0, atunci resetez toata memoria
	if ((*nr_elemente) <= 0) {
		(*memorie) = (Memorie*)calloc(1, sizeof(Memorie));
		(*vect_PID) = (int*)calloc(32768, sizeof(int));
		(*coada_asteptare) = (Queue*)calloc(1, sizeof(Queue));
	}
	else {
		// Altfel, caut elementul in memorie si sar peste el.
		if (((proces*)aux->info)->PID == PID) {
			(*memorie) = (*memorie)->urm;
			return;
		}
		else {
			if (PID == ((proces*)aux->urm->info)->PID) {
				aux->urm = aux->urm->urm;
				return;
			}
			aux = aux->urm;
			while (aux->urm != NULL) {
				if (PID == ((proces*)aux->urm->info)->PID) {
					aux->urm = aux->urm->urm;
					return;
				}
				aux = aux->urm;
			}
			aux = NULL;
		}
	}
}

void run(int timp, int cuantum_timp, int *memorie_totala, Queue **coada_asteptare, celula **running, Queue **coada_finished, Memorie **memorie, int **vect_PID, int *nr_elemente, FILE* output) {
	celula* aux = (*running);
	celula* aux_nou = (*running);
	if (aux != NULL) {
		while (timp > 0) {
			if (aux != NULL) {
				// In acest caz verific daca a mai ramas timp de executat diferit de cuantum_timp
				// Ramas de la un run anterior
				if (cuantum_timp > ((proces*)aux->info)->cuantum_timp) {
					// Daca timpul este mai mare decat cuantum_timp, atunci putem sa rulam cuantum timpul ramas in aceasta celula
					((proces*)aux->info)->timp_executat += ((proces*)aux->info)->cuantum_timp;
					((proces*)aux->info)->timp_executie -= ((proces*)aux->info)->cuantum_timp;
					timp -= ((proces*)aux->info)->cuantum_timp;
					((proces*)aux->info)->cuantum_timp = cuantum_timp;
					// Daca am mai ramas cu timp de executie, atunci il bagam in coada de asteptare si scoatem un nou running
					if (((proces*)aux->info)->timp_executie > 0) {
						aux_nou = aux;
						if ((*coada_asteptare)->front != NULL && (*coada_asteptare)->rear != NULL) {
							aux = extrQ(*coada_asteptare);
							introducereQ(coada_asteptare, (proces*)aux_nou->info, output);
						}
					}
					// Daca nu mai avem timp de executie, atunci il bagam in coada de finished, il scoatem din memorie si scoatem un nou running
					else {
						(*memorie_totala) -= ((proces*)aux->info)->mem_size;
						remove_memorie(((proces*)aux->info)->PID, coada_asteptare, memorie, vect_PID, nr_elemente);
						intrQ(coada_finished, (proces*)aux->info, sizeof(proces));
						aux = extrQ(*coada_asteptare);
						if ((*nr_elemente) == 0) {
							(*running) = NULL;
							return;
						}
					}
				}
				// Daca timpul este mai mic decat cuantum_timp, atunci voi executa doar cat am nevoie
				if (timp < cuantum_timp) {
					// Verific daca scazand din running, termin procesul sau nu
					if (timp - (((proces*)aux->info)->timp_executie) == 0) {
						((proces*)aux->info)->timp_executie = 0;
						((proces*)aux->info)->timp_executat += timp;
						((proces*)aux->info)->cuantum_timp = cuantum_timp;
						(*memorie_totala) -= ((proces*)aux->info)->mem_size;
						remove_memorie(((proces*)aux->info)->PID, coada_asteptare, memorie, vect_PID, nr_elemente);		
						intrQ(coada_finished, (proces*)aux->info, sizeof(proces));
						aux = extrQ(*coada_asteptare);
						if ((*nr_elemente) == 0) {
							(*running) = NULL;
							return;
						}
					}
					else if (timp - (((proces*)aux->info)->timp_executie) < 0) {
						((proces*)aux->info)->timp_executie -= timp;
						((proces*)aux->info)->timp_executat += timp;
						((proces*)aux->info)->cuantum_timp -= timp;
						break;
					}
					else {
						((proces*)aux->info)->timp_executat += ((proces*)aux->info)->timp_executie;
						timp -= ((proces*)aux->info)->timp_executie;
						((proces*)aux->info)->timp_executie = 0;
						(*memorie_totala) -= ((proces*)aux->info)->mem_size;
						remove_memorie(((proces*)aux->info)->PID, coada_asteptare, memorie, vect_PID, nr_elemente);
						intrQ(coada_finished, (proces*)aux->info, sizeof(proces));
						aux = extrQ(*coada_asteptare);
						if ((*nr_elemente) == 0) {
							(*running) = NULL;
							return;
						}
					}
				}
				// Altfel, voi merge din cuantum_timp in cuantum_timp, si verific mereu daca scazand din running, acesta se termina sau nu
				else if (timp >= cuantum_timp) {
					if (cuantum_timp - (((proces*)aux->info)->timp_executie) == 0) {
						((proces*)aux->info)->timp_executie = 0;
						((proces*)aux->info)->timp_executat += cuantum_timp;
						((proces*)aux->info)->cuantum_timp = cuantum_timp;
						(*memorie_totala) -= ((proces*)aux->info)->mem_size;
						remove_memorie(((proces*)aux->info)->PID, coada_asteptare, memorie, vect_PID, nr_elemente);
						intrQ(coada_finished, (proces*)aux->info, sizeof(proces));
						aux = extrQ(*coada_asteptare);
						timp -= cuantum_timp;
						if ((*nr_elemente) == 0) {
							(*running) = NULL;
							return;
						}
					}
					else if (cuantum_timp - (((proces*)aux->info)->timp_executie) < 0) {
						((proces*)aux->info)->timp_executie -= cuantum_timp;
						((proces*)aux->info)->timp_executat += cuantum_timp;
						((proces*)aux->info)->cuantum_timp = cuantum_timp;
						timp -= cuantum_timp;
						aux_nou = aux;
						if ((*coada_asteptare)->front != NULL && (*coada_asteptare)->rear != NULL) {
							aux = extrQ(*coada_asteptare);
							introducereQ(coada_asteptare, (proces*)aux_nou->info, output);
						}
					}
					else {
						((proces*)aux->info)->timp_executat += ((proces*)aux->info)->timp_executie;
						timp -= ((proces*)aux->info)->timp_executie;
						((proces*)aux->info)->timp_executie = 0;
						(*memorie_totala) -= ((proces*)aux->info)->mem_size;
						remove_memorie(((proces*)aux->info)->PID, coada_asteptare, memorie, vect_PID, nr_elemente);
						intrQ(coada_finished, (proces*)aux->info, sizeof(proces));
						aux = extrQ(*coada_asteptare);
						if ((*nr_elemente) == 0) {
							(*running) = NULL;
							return;
						}
					}
				}
			}
			else break;
		}	
		(*running) = aux;
	}
}

void finish(Queue * coada_asteptare, celula * running, FILE* output) {
	// Parcurg coada de asteptare si adun timpii de executie + timpul dee executie al starii running
	int total_time = 0;
	total_time += ((proces*)running->info)->timp_executie;
	celula* aux = coada_asteptare->front;
	while (vidaQ(coada_asteptare) == 0) {
		aux = extrQ(coada_asteptare);
		total_time += ((proces*)aux->info)->timp_executie;
	}
	fprintf(output,  "Total time: %d\n", total_time);
}

void pushS(Memorie **memorie, int PID, int data, FILE* output) {
	Memorie* aux = (*memorie);
	while (aux != NULL) {
		if(aux->info!=NULL)
			if (((proces*)aux->info)->PID == PID) {
				if (sizeof(int) + ((proces*)aux->info)->stack_size > ((proces*)aux->info)->mem_size) {
					fprintf(output, "Stack overflow PID %d.\n", PID);
					return;
				}
				((proces*)aux->info)->stack_size += sizeof(int);
				celStiva* informatie = (celStiva*)calloc(1, sizeof(celStiva));
				informatie->info = data;
				if (vidaS(((proces*)aux->info)->stiva) == 1) {
					((proces*)aux->info)->stiva->top = informatie;
				}
				else {
					informatie->urm = ((proces*)aux->info)->stiva->top;
					((proces*)aux->info)->stiva->top = informatie;
				}
				break;
			}
		aux = aux->urm;
	}
	if (aux == NULL) fprintf(output,  "PID %d not found.\n", PID);
}

void printStack(Memorie **memorie, int PID, FILE* output) {
	Memorie* aux = (*memorie);
	while (aux != NULL) {
		if (((proces*)aux->info)->PID == PID) {
			int i = 0;
			if (vidaS(((proces*)aux->info)->stiva) == 1) fprintf(output,  "Empty stack PID %d.\n", PID);
			else {
				Stack* stack_aux = initS(sizeof(int));
				int nr_elemente = 0;
				fprintf(output, "Stack of PID %d: ", PID);

				while (vidaS(((proces*)aux->info)->stiva) == 0) {
					celStiva* cel = popS(((proces*)aux->info)->stiva);
					push(stack_aux, cel->info);
					nr_elemente++;
					if (nr_elemente == 1) stack_aux->top->urm = NULL;
				}
				while (vidaS(stack_aux) == 0) {
					celStiva* cel = popS(stack_aux);
					nr_elemente--;
					if (nr_elemente == 0) fprintf(output, "%d.\n", cel->info);
					else fprintf(output,"%d ", cel->info);
					push(((proces*)aux->info)->stiva, cel->info);
				}
			}
			return;
		}
		aux = aux->urm;
	}
	fprintf(output, "PID %d not found.\n", PID);
}

void pop(Memorie **memorie, int PID, FILE* output) {
	Memorie* aux = (*memorie);
	while (aux != NULL) {
		if (((proces*)aux->info)->PID == PID) {
			if (((proces*)aux->info)->stiva->top == NULL) fprintf(output,  "Empty stack PID %d.\n", PID);
			else {
				((proces*)aux->info)->stiva->top = ((proces*)aux->info)->stiva->top->urm;
				((proces*)aux->info)->stack_size -= sizeof(int);
			}
			break;
		}
		aux = aux->urm;
	}
	if(aux == NULL) fprintf(output,  "PID %d not found.\n", PID);
}

int main(int argc, int *argv[]) {
	int cuantum_timp, memorie_totala = 0, nr_elemente = 0;
	FILE* intrare = fopen(argv[1], "r");
	FILE* iesire = fopen(argv[2], "w");
	fscanf(intrare, "%d", &cuantum_timp);

	Queue* coada_asteptare = initQ(sizeof(Queue));
	Queue* coada_finished = initQ(sizeof(Queue));
	int* vect_PID = (int*)calloc(32768, sizeof(int));
	celula* running = (celula*)calloc(1, sizeof(celula));
	Memorie* memorie = (Memorie*)calloc(1, sizeof(Memorie));

	int stare_running = 0;

	char optiune[50];

	while (!feof(intrare)) {
		fgets(optiune, 50, intrare);
		char* p = (char*)malloc(30);
		p = strtok(optiune, " ");
		if (strcmp(p, "add") == 0) {
			p = strtok(NULL, " ");
			int mem_size = atoi(p);
			p = strtok(NULL, " ");
			int exec_time = atoi(p);
			p = strtok(NULL, " ");
			int prioritate = atoi(p);
			add(&coada_asteptare, &memorie, &nr_elemente, &memorie_totala, mem_size, exec_time, prioritate, cuantum_timp, &vect_PID, &stare_running, &running, iesire);
		}
		if (strcmp(p, "get") == 0) {
			p = strtok(NULL, " ");
			int PID = atoi(p);
			get(PID, coada_asteptare, running, coada_finished, iesire);
		}
		if (strcmp(p, "print") == 0) {
			p = strtok(NULL, " ");
			if (p[strlen(p) - 1] == '\n')
				p[strlen(p) - 1] = '\0';
			if (strcmp(p, "finished") == 0)
				print_finished(coada_finished, iesire);
			else if (strcmp(p, "waiting") == 0)
				print_waiting(coada_asteptare, iesire);
			else if (strcmp(p, "stack") == 0) {
				p = strtok(NULL, " ");
				int PID = atoi(p);
				printStack(&memorie, PID, iesire);
			}
		}
		if (strcmp(p, "run") == 0) {
			p = strtok(NULL, " ");
			int timp = atoi(p);
			if(coada_asteptare!=NULL)run(timp, cuantum_timp, &memorie_totala, &coada_asteptare, &running, &coada_finished, &memorie, &vect_PID, &nr_elemente, iesire);
		}
		if (strcmp(p, "finish\n") == 0) {
			finish(coada_asteptare, running, iesire);
			return 0;
		}
		if (strcmp(p, "push") == 0) {
			p = strtok(NULL, " ");
			int PID = atoi(p);
			p = strtok(NULL, " ");
			int data = atoi(p);
			pushS(&memorie, PID, data, iesire);
		}
		if (strcmp(p, "pop") == 0) {
			p = strtok(NULL, " ");
			int PID = atoi(p);
			pop(&memorie, PID, iesire);
		}
	}
	return 0;
}
