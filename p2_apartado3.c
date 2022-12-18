#include <stdio.h>
#include <stdlib.h>
#include <pmmintrin.h>
#include <time.h>
#include <unistd.h>
#include <immintrin.h>

#define N 250
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
	double **a,**b,*c; //matrices y vector que almacenan valores aleatorios de tipo double
	double **d;
	double f;					//variable de salida de tipo double
	int *ind;
	int aux[N];
	int cnt;
	double *e;
	double ck;
	/*DECLARACIÓN DE LOS DIFERENTES REGISTROS*/
	__m128d calA,calB,calC,reg2;
	double dos[] = {2.0,2.0};		//Inicialización del vector de doses
	reg2 = _mm_load_pd(dos);		//Cargamos el registro de doses.

	/*CREACIÓN DE LAS DIFERENTES MATRICES Y VECTORES*/
	//Estos son alineados para que no se produzcan errores
	a = (double **)_mm_malloc(N*sizeof(double*),16);
		for(int i = 0; i < N ; i++){
			a[i] = (double*)malloc(C*sizeof(double));
		}

		b = (double **)_mm_malloc(C*sizeof(double*),16);
		for(int i = 0; i < C; i++){
			b[i] = (double *)malloc(N*sizeof(double));
		}

		c = (double*)_mm_malloc(C*sizeof(double),16);

		e = (double *)_mm_malloc(N*sizeof(double),16);

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


		d = (double **)_mm_malloc(N*sizeof(double *),16);
		for(int i = 0; i < N; i++){
			d[i] = (double*)malloc(N*sizeof(double));
		}


		srand(156798);
		/*INICIALIZACIÓN DE LAS DIFERENTES MATRICES CON NÚMEROS ALEATORIOS*/
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
		for(int i = 0; i < N; i++){
			for(int j = 0; j < N; j++){
				d[i][j] = 0;
			}
		}

		double temporal[2];
	//COMPUTACIÓN
	start_counter();
	/*CARGA DE REGISTROS Y CÁLCULO DE LA MATRIZ D*/
	for(int64_t i = 0; i < N ; i++){
		for(int64_t j = 0; j < N; j++){
			for(int64_t k = 0; k < C; k+= 2){
				calA = _mm_load_pd(&a[i][k]);	//Cargamos dos doubles de A
				temporal[0] = b[k][j];
				temporal[1]  = b[k+1][j];
				calB = _mm_load_pd(temporal);	//Cargamos dos doubles de B
				calC = _mm_load_pd(&c[k]);		//Cargamos dos doubles de C
				calA = _mm_mul_pd(reg2, calA);	//Multiplicamos 2*la carga del registro de A
				calC = _mm_sub_pd(calB, calC);	//Restamos los registros de B y C
				calA = _mm_mul_pd(calA, calC);	//Multiplicamos finalmente los últimos dos registros calculados
				_mm_store_pd(temporal, calA);	//Almacenamos los valores del registro A en el vector temporal
				d[i][j] += temporal[0]+ temporal[1];	//Suma de los diferentes elementos del vector temporal en la posición de d
			}
		}
	}

	f = 0;
	/*CÁLCULO DE LA VARIABLE RESULTADO*/
	for(int i = 0; i < N; i++){
		e[i] = d[ind[i]][ind[i]]/2;
		f+=e[i];
	}

	printf("F: %lf\n",f);
	ck = get_counter();
	printf("Counter: %1.2lf, ",ck);
	mhz(1, 1);


	/*LIBERACIÓN DE LAS MATRICES*/
	for(int i = 0; i < N; i++){
		free(d[i]);
	}
	_mm_free(d);

	for(int i = 0; i < N; i++){
		free(a[i]);
	}
	_mm_free(a);

	for(int i = 0; i < 8; i++){
		free(b[i]);
	}
	_mm_free(b);

	/*LIBERACIÓN DE LOS DIFERENTES VECTORES*/
	_mm_free(c);
	free(ind);
	_mm_free(e);
}

