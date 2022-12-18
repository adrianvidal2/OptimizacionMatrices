#include <stdio.h>
#include <stdlib.h>
#include <pmmintrin.h>
#include <time.h>
#include <unistd.h>


#define N 3000
#define C 8


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
	double **a,**b,*c; 			//matrices y vector que almacenan valores aleatorios de tipo double
	double **d;					//matriz resultado
	double f;					//variable de salida de tipo double
	int *ind;					//vector almacenador de índices aleatorios
	int aux[N];
	int cnt;
	double *e;
	double ck;

	/*RESERVAS DE MEMORIA PARA LAS MATRICES Y VECTORES*/
	e = (double *)malloc(N*sizeof(double));
	for(int i = 0; i < N; i++){
		aux[i] = 0;
		e[i] = 0;
	}
	a = (double **)malloc(N*sizeof(double*));
	for(int i = 0; i < N; i++){
		a[i] = (double*)malloc(C*sizeof(double));
	}

	b = (double **)malloc(C*sizeof(double*));
	for(int i = 0; i < C ; i++){
		b[i] = (double*)malloc(N*sizeof(double));
	}

	c = (double*)malloc(C*sizeof(double));

	srand(156798);
	f = 0;
	/*INICIALIZACIÓN DE LAS MATRICES ALEATORIAMENTE*/
	for(int i = 0; i < N; i++){
		for(int j = 0; j < C; j++){
			a[i][j] = (rand() % N);
		}
	}

	for(int i = 0; i < C; i++){
		for(int j = 0; j < N; j++){
			b[i][j] = (rand() % N);
		}
	}

	for(int i = 0; i < C; i++){
		c[i] = (rand() % N);
	}

	ind = (int*)malloc(N*sizeof(int));	//Reservamos e inicializamos el vector ind (índices barajados aleatoriamente)
		for(int i = 0; i < N;){
				cnt = (rand() % N);
		        if(!aux[cnt]){
		        	ind[i] = cnt;
		        	aux[cnt] = 1;
		        	i++;
		        }
		 }

		/*CREACIÓN E INICIALIZACIÓN DE LA MATRIZ RESULTADO*/
		d = (double **)malloc(N*sizeof(double*));	//Creación de matriz d
			for(int i = 0; i < N; i++){
				d[i] = (double*)malloc(N*sizeof(double));
			}

			for(int i = 0; i < N; i++){					//Inicialización de d a 0
				for(int j = 0; j < N;j++){
					d[i][j] = 0;

				}
			}

	//COMPUTACIÓN
	start_counter();

	/*CÁLCULO DE LA MATRIZ D*/
	for(int i = 0; i < N; i++){
		for(int j = 0; j < N; j++){
			for(int k = 0; k < C; k++){
				d[i][j] += 2*a[i][k] * (b[k][j] - c[k]);
			}
		}
	}

	f = 0;
	/*CÁLCULO DE LA VARIABLE FINAL*/
	for(int i = 0; i < N; i++){
		e[i] = d[ind[i]][ind[i]]/2;
		f+=e[i];
	}

	printf("F: %lf\n",f);
	ck = get_counter();
	printf("Counter: %1.2lf, ",ck);
	mhz(1, 1);


	/*LIBERACIÓN DE MATRICES*/
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

	/*LIBERACIÓN DE VECTORES*/
	free(c);
	free(ind);
	free(e);
}
