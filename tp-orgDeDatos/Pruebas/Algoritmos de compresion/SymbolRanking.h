#ifndef SYMBOLRANKING_H
#define SYMBOLRANKING_H

#include <iostream>
#include <string>
#include <algorithm>
#include <functional>
#include <unordered_map>
#include <list>
#include <tuple>

#include "WFC.h"

using namespace std;

class SymbolRanking{
	private:
		char tabla[255]; //Con los valores ASCII no se si usa.
		WFC wfc;
		list<unsigned long> exclusionList;
		int orden; //Orden maximo de contexto.

		class pairHash{
		public:
		    size_t operator()(const string &k) const{
		    	const char* chars = k.c_str();
		    	return chars[0]*255+chars[1];
		    }
		};

		//Hash (tambien llamado map) con 255*255 claves que son combinacion de dos char. Sus valores
		// son listas de las posiciones de sus apariciones de dichas claves en el buffer.
		unordered_map<string, list<unsigned long>, pairHash> mymap;


		/* Dado un orden de contexto y un char busca en el vector y devuelve
		* true or false segun si lo encontro o no y el numero de no ocurrencias.
		* EJ: mejor caso (true,0) peor caso (false, contextosQueSeComparan).
		* Sigue el esquema de hashing presentado en el Tech paper 132 de P. Fenwich
		*/
		tuple<bool,unsigned short> buscarEnContexto(int orden, char caracter,unsigned long pos, char* buffer);

		/* Realiza la busqueda del caracter para el caso de contexto = 1. */
		tuple<bool,unsigned short> buscarEnContextoUno(char caracter, unsigned long pos, char* buffer);

		/* Dadas dos offset de un buffer, se comparan N Bytes (N = nro de orden).
		 * Si dichos Bytes son iguales, entonces dichos strings son iguales y por lo tanto los contextos son iguales.
		 * Precondiciones:
		 *  -indexA e indexB deben ser offsets validos del buffer, donde indexA < indexB
		 * Postcondiciones:
		 *  Devuelve un boolean de acuerdo a si se produce un match de contextos o no
		 */
		bool contextosIguales(unsigned long indexA, unsigned long indexB, char* buffer,int ordenDelContexto);

		/*
		 * Dado un offset de un array, verifica si el char apuntado por esa posicion es el caracter a comparar.
		 * Precondiciones:
		 *  - index debe ser un indice valido del buffer
		 * Postcondiciones:
		 *  Devuelve un boolean de acuerdo a si hay un match entre los caracteres involucrados
		 */
		bool charsIguales(unsigned long index,char charToCompare,char* buffer);

		/* Busca si el caracter ofrecido como ranking esta en la lista de exclusion.
		 * Postcondiciones:
		 *  devuelve True si no se encuentra en la lista.
		 */
		bool charNoExcluido(unsigned long pos);

		/* Dado un vector con 2 char, lo hashea en el map (atributo).
		 */
		void hashear(char symbol1, char symbol2, unsigned long indexFirstChar);

		// No estoy seguro.
		list<tuple<unsigned long,char>> hasheartu(char symbol1, char symbol2, unsigned long indexFirstChar);

	public:
		SymbolRanking();

		SymbolRanking(int orden);

		/*Recibe un vector de char a comprimir. El resultado se almacenara en un vector de short (necesito mas de 256 caracteres
		 * por lo que no se puede guardar en un vector de char */
		void comprimir(char* aComprimir, short* salida, unsigned long size);

		/*
		 * Recibe un vector de shorts y genera un vector de chars descomprimidos.
		 */
		void descomprimir(short* aDescomprimir, char* salida, unsigned long size);
};

#endif


