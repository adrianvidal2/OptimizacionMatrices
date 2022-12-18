#include <stdio.h>
#include <stdlib.h>
#include <pmmintrin.h>
#include <time.h>
#include <unistd.h>
#include <omp.h>

#define N 500
#define C 8
#define k 6

void start_counter();
double get_counter();
double mhz();


	/* Initialize the cycle counter */


	 static unsigned cyc_hi = 0;
	 static unsigned cyc_lo = 0;


	 /* Set *hi and *lo to the high and low order bits of the cycle counter.
	 Implementation requires assembly code to use the rdtsc instruction. */
void access_counter(unsigned *hi, unsigned *lo)
	 {
	 asm("rdtsc; movl %%edx,%0; movl %%eax,%1" /* Read cycle counter */
	 : "=r" (*hi), "=r" (*lo) /* and move results to */
	 : /* No input */ /* the two outputs */
	 : "%edx", "%eax");
}

	 /* Record the current value of the cycle counter. */
void start_counter(){
	 access_counter(&cyc_hi, &cyc_lo);
}

	 /* Return the number of cycles since the last call to start_counter. */
double get_counter(){
	 unsigned ncyc_hi, ncyc_lo;
	 unsigned hi, lo, borrow;
	 double result;

	 /* Get cycle counter */
	 access_counter(&ncyc_hi, &ncyc_lo);

	 /* Do double precision subtraction */
	 lo = ncyc_lo - cyc_lo;
	 borrow = lo > ncyc_lo;
	 hi = ncyc_hi - cyc_hi - borrow;
	 result = (double) hi * (1 << 30) * 4 + lo;
	 if (result < 0) {
	 fprintf(stderr, "Error: counter returns neg value: %.0f\n", result);
	 }
	 return result;
}

double mhz(int verbose, int sleeptime){
	 double rate;

	 start_counter();
	 sleep(sleeptime);
	 rate = get_counter() / (1e6*sleeptime);
	 if (verbose)
	 printf("\n Processor clock rate = %.1f MHz\n", rate);
	 return rate;
}



int main(int argc, char **argv) {
	double **a,**b,*c,**prima; //matrices y vector que almacenan valores aleatorios de tipo double
	double **d;
	double f;					//variable de salida de tipo double
	int *ind;
	int aux[N];
	int cnt;
	double *e;
	double ck;

	/*CREACIÓN DE LAS MATRICES Y VECTORES*/
	a = (double **)malloc(N*sizeof(double*));
	for(int i = 0; i < N ; i++){
		a[i] = (double*)malloc(C*sizeof(double));
	}

	b = (double **)malloc(C*sizeof(double*));
	for(int i = 0; i < C; i++){
		b[i] = (double *)malloc(N*sizeof(double));
	}

	c = (double*)malloc(C*sizeof(double));

	e = (double *)malloc(N*sizeof(double));

	for(int i = 0; i < N; i++){
		aux[i] = 0;
		e[i] = 0;
	}

	ind = (int *)malloc(N*sizeof(int));
	for(int i = 0; i < N;){
		cnt = (rand() % N);
		if(!aux[cnt]){
		    ind[i] = cnt;
			aux[cnt] = 1;
			i++;
		}
	}


	d = (double **)malloc(N*sizeof(double *));
	for(int i = 0; i < N; i++){
		d[i] = (double*)malloc(N*sizeof(double));
	}



	srand(156798);
	/*INICIALIZACIÓN DE LAS DIFERENTES MATRICES Y VECTORES DE MANERA ALEATORIA*/
	for(int i = 0; i < N; i++){
		for(int j = 0; j < C; j++){
			a[i][j] = (rand()%N);
		}
	}

	for(int i = 0; i < C; i++){
		for(int j = 0; j < N; j++){
			b[i][j] = (rand()%N);
		}
	}

	for(int i = 0; i < C; i++){
		c[i] = (rand()%N);
	}

	f = 0;

	prima = (double**)malloc(C*sizeof(double*));

	for(int i = 0;i < C; i++){
		prima[i] = (double*)malloc(N*sizeof(double));
	}
	for(int i = 0; i < C; i++){
		for(int j = 0; j < N; j++){
			prima[i][j] = 0;
		}
	}
	//COMPUTACIÓN
	start_counter();

	/*
	 * Establecemos o pragma coas diferentes variables compartidas e o número de fíos que se utilizarán
	 * Definimos unha variable f_privado que nos axudará a evitar a carreira crítica que se produciría
	 * cando os fíos intentasen escribir o resultado de f.
	 */
#pragma omp parallel shared(prima,a,b,c,d,f,e) num_threads(k)
	{
		double f_privado = 0;		//variable auxiliar para evitar carreras críticas.
		/*PARALELIZACIÓN DE LA ZONA*/
		#pragma omp for
		for(int j = 0; j < N; j++){
			prima[0][j] += b[0][j] - c[0];
			prima[1][j] += b[1][j] - c[1];
			prima[2][j] += b[2][j] - c[2];
			prima[3][j] += b[3][j] - c[3];
			prima[4][j] += b[4][j] - c[4];
			prima[5][j] += b[5][j] - c[5];
			prima[6][j] += b[6][j] - c[6];
			prima[7][j] += b[7][j] - c[7];
		}

		/*CÁLCULO DE LA MATRIZ D*/
		#pragma omp for
		for(int i = 0; i < N; i++){
			for(int j = 0; j < N; j++){
				d[i][j] = 2*a[i][0] * prima[0][j];
				d[i][j] += 2*a[i][1] * prima[1][j];
				d[i][j] += 2*a[i][2] * prima[2][j];
				d[i][j] += 2*a[i][3] * prima[3][j];
				d[i][j] += 2*a[i][4] * prima[4][j];
				d[i][j] += 2*a[i][5] * prima[5][j];
				d[i][j] += 2*a[i][6] * prima[6][j];
				d[i][j] += 2*a[i][7] * prima[7][j];
			}
		}

		#pragma omp for
		for(int i = 0; i < N; i++){
				e[i] = d[ind[i]][ind[i]]/2;
				f_privado+=e[i];
		}
		#pragma omp atomic
		f += f_privado;
	}
	printf("F: %lf\n",f);
	ck = get_counter();
	printf("Counter: %1.2lf, ",ck);
	mhz(1, 1);


	/*LIBERACIÓN DE LAS MATRICES*/
		for(int i = 0; i < N; i++){
			free(d[i]);
		}
		free(d);

		for(int i = 0; i < N; i++){
			free(a[i]);
		}
		free(a);

		for(int i = 0; i < 8; i++){
			free(b[i]);
		}
		free(b);

		/*LIBERACIÓN DE LOS VECTORES*/
		free(c);
		free(ind);
		free(e);
}


