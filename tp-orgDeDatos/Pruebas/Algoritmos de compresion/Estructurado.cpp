#include "Estructurado.h"

typedef struct _par{
    int numero;
    int ocurrencias;
} par_t;

struct _nivel{
    unsigned long int total_ocurrencias;
    list<par_t*> cant_por_nro;
    unsigned short numeroMaximoDelNivel;
};

par_t* par_crear(int nro){
    par_t* par_nro = new par_t;
    par_nro->numero = nro;
    par_nro->ocurrencias = OCURRENCIAS_INICIAL;
    return par_nro;
}

nivel_t& nivel_crear(int nro_nivel){
    nivel_t* nivel = new nivel_t;

    nivel->cant_por_nro.push_back(par_crear(NRO_ESCAPE));
    unsigned long int total_ocurrencias = 1;

    if (nro_nivel == 0){
        nivel->cant_por_nro.push_back(par_crear(0));
        nivel->numeroMaximoDelNivel = 0;
        total_ocurrencias++;
    } else {
        for (int i = pow(2, nro_nivel-1); i < pow(2, nro_nivel); i++){
            nivel->cant_por_nro.push_back(par_crear(i));
            nivel->numeroMaximoDelNivel = pow(2, nro_nivel)-1;
            total_ocurrencias++;
        }
    }
    nivel->total_ocurrencias = total_ocurrencias;
    return *nivel;
}

void nivel_destruir(nivel_t& nivel){
    nivel.cant_por_nro.clear();
    delete &nivel;
}

Estructurado::Estructurado(){
	resultado = new string;
    high = 0xffff; //16 bits
    low = 0x0000;  //16 bits
    underflow = 0;

    niveles = new nivel_t[CANT_NIVELES];
    for (int i = 0; i < CANT_NIVELES; i++){
        nivel_t& nuevo = nivel_crear(i);
        niveles[i] = nuevo;
    }
}

Estructurado::~Estructurado(){
//    for (int i = 0; i < CANT_NIVELES; i++)
//        nivel_destruir(niveles[i]);
    delete[] niveles;
}

void Estructurado::emitirEscape(int nivel, int i){
    emitirNro(nivel, NRO_ESCAPE, i);
}

void Estructurado::emitirBit(bool bit){
	*resultado += bit;
}

void Estructurado::emitirEOF(int j){
    for (int i = 0; i < CANT_NIVELES; i++)
        emitirEscape(i, j);
}

pair<char*, unsigned int> Estructurado::generar_resultado(){
	char* salida = new char[resultado->length()/8];
	for (unsigned int i=0; i<resultado->length()/8; ++i){
		char aGuardar = 0x00;
		for(int j = 0; j<8; j++, i++){
			aGuardar |= salida[i];
			aGuardar <<= 1;
		}
		salida[i/8] = aGuardar;
	}
	pair <char*, unsigned int> par (salida, resultado->length()/8);
	return par;
}

pair<char*, unsigned int> Estructurado::comprimir(short* aComprimir, unsigned int size){
    for (unsigned int i = 0; i < size; i++){
        int nivel_indice;
        unsigned short numeroAComprimir = aComprimir[i];

        if(numeroAComprimir == 0) nivel_indice = 0;
        else nivel_indice = int(log2(numeroAComprimir)) + 1;

        int nivel_act = NIVEL_INICIAL;
        for (; nivel_act < nivel_indice; nivel_act++){
            emitirEscape(nivel_act, i);
        }
        emitirNro(nivel_act, numeroAComprimir, i);
    }
    emitirEOF(0);
    return generar_resultado();
}

void Estructurado::emitirNro(int nro_nivel, int nro, int i){
    nivel_t& nivel = niveles[nro_nivel];
    int nro_sig;
    if ((nro_nivel <= 2)&&(nro == NRO_ESCAPE)) nro_sig = nro_nivel;
    else nro_sig = (nro == NRO_ESCAPE) ? (pow(2, nro_nivel-1)) : nro+1;
    unsigned short frecuenciaTechoDelNumeroAComprimir = frecuenciaAcumuladaHastaElNumero(nivel,nro_nivel,nro_sig, i);
    unsigned short frecuenciaTechoDelNumeroPrevioAComprimir = frecuenciaAcumuladaHastaElNumero(nivel,nro_nivel,nro, i); //Piso del numero a comprimir
    unsigned short frecuenciaTotal = nivel.total_ocurrencias;

    unsigned int range = (high-low) + 1;
    high = low + ((range*frecuenciaTechoDelNumeroAComprimir)/frecuenciaTotal)-1;
    low = low + ((range*frecuenciaTechoDelNumeroPrevioAComprimir)/frecuenciaTotal);

    //Caso de fin de archivo
    if ((nro_nivel == (CANT_NIVELES-1)) && (nro == NRO_ESCAPE)){
        finalizarCompresion(low);
    }

    //Ciclo hasta que no hayan mas bits para que sean emitidos o para aplicar shifts
    while(true){
        bool msbOfHigh = ((high >> 15) != 0);   //Efectua la comparacion para convertir el short en un bool
        bool msbOfLow = ((low >> 15) != 0);
        bool secondMsbOfHigh = (((high & 0x4000) >> 14) != 0);
        bool secondMsbOfLow = (((low & 0x4000) >> 14) != 0);

        if (msbOfHigh == msbOfLow){
            emitirBit(msbOfLow);
            while(underflow>0){
                emitirBit(~msbOfLow);
                underflow--;
            }
        }
        // Contemplar Underflow
        else if((secondMsbOfLow==1)&&(secondMsbOfHigh==0)){
            underflow++;
            low = low & 0x3fff;
            high = high | 0x4000;
        }
        // Si no hay msb iguales ni tampoco Underflow, sale del ciclo
        else break;
        // Realiza los shift finales
        high = (high << 1) | 0x0001;
        low = low << 1;
    }
    //Actualizar las frecuencias
    incrementarFrecuencias(nivel,nro);
}

unsigned short Estructurado::frecuenciaAcumuladaHastaElNumero(nivel_t& nivel,int nro_nivel,int nro, int i){
    unsigned short frecuenciaPisoDelNumero=0; //Con el verificarFrecuencias, nos aseguramos que no tendremos frecuencias mayores a 16 bits

    //Caso de que se pida la frec del max numero del nivel
    if(nro == (nivel.numeroMaximoDelNivel + 1)) return nivel.total_ocurrencias;

    //Caso generico
    list<par_t*>::iterator it = nivel.cant_por_nro.begin();
    for (; (*it)->numero != nro; it++){
        frecuenciaPisoDelNumero += (*it)->ocurrencias;
    }
    return frecuenciaPisoDelNumero;
}

void Estructurado::incrementarFrecuencias(nivel_t& nivel, int nro){
    // Incrementa las ocurrencias totales del nivel
    nivel.total_ocurrencias++;

    // Incrementa la frecuencia de nro en el nivel determinado
    list<par_t*>::iterator it = nivel.cant_por_nro.begin();
    for (; (*it)->numero != nro; it++); // Hace que el iterator se posicione en la posicion del nro
    (*it)->ocurrencias++;
    verificarFrecuencias(nivel);
}

void Estructurado::finalizarCompresion(unsigned short low){
    //Emite msb de Low
    bool msbOfLow = ((low >> 15) != 0);
    emitirBit(msbOfLow);

    //Emite underflows
    while(underflow>0){
        emitirBit(~msbOfLow);
        underflow--;
    }

    //Emite los demas bits del low
    for(short shift = 14;shift<0;shift--){
        unsigned short mask = (2^(shift+1))-1;
        bool bit = (bool) (low & mask) >> shift;
        emitirBit(bit);
    }
}

void Estructurado::verificarFrecuencias(nivel_t& nivel){

	//Si no se supera el limite de frecuencias, salimos del metodo
	if (nivel.total_ocurrencias < LIMITE_FRECUENCIAS) return;

	unsigned short frecuenciasTotales=0;

	//Si se supera el limite, procedemos a normalizar las frecuencias del nivel, reduciendolas a la mitad
    list<par_t*>::iterator numeroDelNivel = nivel.cant_por_nro.begin();
    for (; numeroDelNivel != nivel.cant_por_nro.end(); numeroDelNivel++){

    	(*numeroDelNivel)->ocurrencias/=2;
    	//SI el numero normalizado queda en cero, lo seteamos a 1
    	if ((*numeroDelNivel)->ocurrencias == 0) (*numeroDelNivel)->ocurrencias = 1;

    	frecuenciasTotales+= (*numeroDelNivel)->ocurrencias;
    }

	//Actualiza las frecuencias totales del nivel
	nivel.total_ocurrencias = frecuenciasTotales;
}


pair<unsigned short*, unsigned int> Estructurado::descomprimir(char* indices, unsigned int size){
	pair <unsigned short*, unsigned int> par;
//    int emitido = NRO_ESCAPE;
//    while(true){
//        int nivel_act = NIVEL_INICIAL;
//        for (; emitido != NRO_ESCAPE; nivel_act++){
//            emitido = obtenerNro(nivel_act, nro_comprimido);
//        }
//        if ((nivel_act == CANT_NIVELES -1) && (emitido == NRO_ESCAPE)) break;
//        indices += emitido;
//    }
    return par;
}

/*
int Estructurado::obtenerNro(int nro_nivel, double nro_comprimido){
    nivel_t& nivel = niveles[nro_nivel];
    int cant_parcial = 0;
    int nuevo_seg_ini = (fin_segmento - inicio_segmento) * cant_parcial / nivel.total_ocurrencias + inicio_segmento;

    list<par_t*>::iterator it = nivel.cant_por_nro.begin();
    for (; nuevo_seg_ini <= nro_comprimido; it++){
        cant_parcial += (*it)->ocurrencias;
        nuevo_seg_ini = (fin_segmento - inicio_segmento) * cant_parcial / nivel.total_ocurrencias + inicio_segmento;
    }
    it--;
    cant_parcial += (*it)->ocurrencias;
    nuevo_seg_ini = (fin_segmento - inicio_segmento) * cant_parcial / nivel.total_ocurrencias + inicio_segmento;
    int nuevo_seg_fin = (fin_segmento - inicio_segmento) * (cant_parcial + (*it)->ocurrencias)/ nivel.total_ocurrencias + inicio_segmento;
    inicio_segmento = nuevo_seg_ini;
    fin_segmento = nuevo_seg_fin;
    (*it)->ocurrencias++;
    nivel.total_ocurrencias++;
    return (*it)->numero;
}

string& Estructurado::descomprimir(double nro_comprimido){
    string& indices = *(new string);
    int emitido = NRO_ESCAPE;
    while(true){
        int nivel_act = NIVEL_INICIAL;
        for (; emitido != NRO_ESCAPE; nivel_act++){
            emitido = obtenerNro(nivel_act, nro_comprimido);
        }
        if ((nivel_act == CANT_NIVELES -1) && (emitido == NRO_ESCAPE)) break;
        indices += emitido;
    }
    return indices;
}
*/
