// Gloton_X_Local_Search01_Consola.cpp : Defines the entry point for the console application.
//

// Viene de Gloton_X_Local_Search01.cpp : Defines the entry point for the console application.


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_ITEMS 1000
#define MAX_DEFECTS 1000
#define MAX_PLATES 120
#define HEIGH_PLATE 3210
#define WIDTH_PLATE 6000
#define MIN_WASTE 20
#define MAX_CORTE_TIPO_UNO 3500
#define MIN_CORTE_TIPO_UNO 100
#define ANT_POSICION_NULA 0 //Esta posicion es cuando no hay nadie anterior a el (es la primera figura del plate)
#define POSICION_DER 1 //Esta posicion es cuando el nuevo item se encuentra a la derecha del anterior
#define POSICION_ARRIBA 2 //Esta posicion es cuando el nuevo item se encuentra arriba del anterior
#define POSICION_NUEVA 3 //Esta posicion es cuando el nuevo item se encuentra en una nueva fila-columna, pero en el mismo plate
#define CIEN_MILLONES 100000000
#define MAX_NIVEL 10
#define EPSILON 0.00001

struct struct_item {
	int id;
	int length,witdth;
	int stack;
	int sequence;
};

struct struct_defect {
	int id;
	int plate;
	int x,y;
	int witdth,heigh;
};

struct vertice {
	int num;
	int id;
	vertice *next;
};

struct nodo {
	int num;
	nodo *next;
};

struct struct_solution_item {
	int x,y;
	int tipo; //Si es original o rotado
	int plate; //Que plate le corresponde
	int id;
	int tipo_new_posicion; //cual es el tipo de posicion del item anterior
};

struct struct_impresion_solucion  {
int plate;
int nodo_id;
int x,y;
int width,heigh;
int type,cut;
int parent;
};

struct struct_backtraking {
	int plate;
	int pos_it1;
	int pos_sol;
	int x_min,y_min;
	int x_max,y_max;
	int x_act;
};

struct struct_info_items_corte_uno {
	int pos_ini;
	int pos_fin;
	int desperdicio;
};

struct struct_sol
{
	int it1;
	int pos_sol;
};

//Items y su lista
struct_item item[MAX_ITEMS];
vertice L_STACK[MAX_ITEMS];
//Defectos
struct_defect defecto[MAX_DEFECTS];
nodo L_DEFECT[MAX_DEFECTS];
int num_items=0,num_defectos=0;
int num_stacks=0,num_plates=0;
//Variables para medir el tiempo
time_t inicio_global,final_global;
//variable para calcular el costo
long int area_items=0,max_costo;
//Variables para crear una solucion 
int bandera[MAX_ITEMS],candidato[MAX_ITEMS];
int array_defect[MAX_ITEMS];
struct_backtraking backtraking[MAX_ITEMS];
// Variables para guardar la solucion
struct_solution_item solution_item[MAX_ITEMS],posible_lugar[MAX_ITEMS],mejor_solucion[MAX_ITEMS];;
long int mejor_costo=-1;
//Variables para imprimir una solucion
struct_impresion_solucion impresion_solucion[MAX_ITEMS*3];
//Variables para llevar el conteo
int primera_solucion=1;
long double suma_acu=0;
//Variables para calcular la cota
int min_p1=200,max_x_cota=0;
//Variable para el limite de la recuperacion
long int lim_itera=10000000;
//long int lim_itera=100000;
//Variables para mejorar a encuentra_pos
int defecto_encontrado[MAX_ITEMS],num_defectos_encontrados;
struct_info_items_corte_uno info_corte_uno[MAX_ITEMS];
int cota_desperdicio=-1;
int arreglo_reco_x[MAX_ITEMS],arreglo_reco_y[MAX_ITEMS];
//Contiene la informacion básica de la solucion actual para que sea construida
struct_sol sol_actual[MAX_ITEMS],sol_respaldo[MAX_ITEMS],res_sol_nivel[MAX_NIVEL][MAX_ITEMS],sol_best[MAX_ITEMS];
int max_nivel=2;
//Variables para calcular los movimientos
int combinacion[MAX_ITEMS],item_combinado[MAX_ITEMS];
int bandera_tabu[MAX_ITEMS];
double porcentaje[MAX_PLATES],porcentaje02[MAX_PLATES];
int max_time=3600;
//Esta funcion recibe una lista y un numero y lo inserta en orden ascendente (de menor a mayor)
void inserta(nodo &L,int num)
{
nodo *p,*q;
p=&L;
q=p->next;
while(q!=NULL && num>q->num)
	{
	p=p->next;
	q=q->next;
	}
if(q==NULL || (q!=NULL && q->num!=num))
	{
	L.num++;
	q=new nodo;
	if(q!=NULL)
		{
		q->next=p->next;
		q->num=num;
		p->next=q;
		}
	else
		printf("Se acabo la memoria");
	}
}

//Esta funcion recibe una lista y un numero y lo inserta en orden ascendente (de menor a mayor)
void inserta(vertice &L,int num,int id)
{
vertice *p,*q;
p=&L;
q=p->next;
while(q!=NULL && num>q->num)
	{
	p=p->next;
	q=q->next;
	}
if(q==NULL || (q!=NULL && q->num!=num))
	{
	L.num++;
	q=new vertice;
	if(q!=NULL)
		{
		q->next=p->next;
		q->num=num;
		q->id=id;
		p->next=q;
		}
	else
		printf("Se acabo la memoria");
	}
}

int strcmp02(char cad01[],char cad02[])
{
int i,ban=0;
for(i=0;cad01[i] && cad02[i] && !ban;++i)	
	if(cad01[i]>cad02[i])
		ban=1;
	else
		if(cad01[i]<cad02[i])
			ban=-1;
if(!ban && cad01[i])
	ban=1;
if(!ban && cad02[i])
	ban=-1;
return ban;
}

int encuentra_entero(int &pos,char cad[])
{
int num=0,ban=0;
//Posiciona la cadena en el primer digito
for(;!ban;++pos)
	if(cad[pos]>='0' && cad[pos]<='9')
		{
		ban=1;
		break;
		}
//Lee el numero
for(;ban && cad[pos];++pos)
	{
	if(cad[pos]>='0' && cad[pos]<='9')
		num=num*10+cad[pos]-'0';
	else
		ban=0;
	}
return num;
}

int parsea_cadena_de_items(char cad[])
{
int pos=0,ban=0,s1;
//Lee el id del item
item[num_items].id=encuentra_entero(pos,cad);
//Lee la longitud del item
item[num_items].length=encuentra_entero(pos,cad);
//Lee la anchura del item
item[num_items].witdth=encuentra_entero(pos,cad);
//Lee la pila del item
item[num_items].stack=encuentra_entero(pos,cad);
//Lee la secuencia del item
item[num_items].sequence=encuentra_entero(pos,cad);
//Inserta el elemento en su lista
s1=item[num_items].stack;
inserta(L_STACK[s1],item[num_items].sequence,num_items);
++num_items;
return ban;
}

int lee_batch(char archivo[])
{
FILE *in;
char entrada[80],cadena[120],cadena_ant[120]="",car;
int i,ban=1;
strcpy(entrada,archivo);
strcat(entrada,"_batch.csv");
in=fopen(entrada,"rb+");
if(in!=NULL)
	{
	//Inicializa las listas de las pilas
	for(i=0;i<MAX_ITEMS;++i)
		{
		L_STACK[i].num=0;
		L_STACK[i].next=NULL;
		}
	info_corte_uno[0].pos_ini=0; //Inicializa la primera posicion del areglo info_corte_uno, (siempre sera 0)
	//Lee la primera linea de encabezados
	fscanf(in,"%[^\n]",cadena);
	fscanf(in,"%c",&car);
	while(!feof(in))
		{
		//Lee una linea de datos
		fscanf(in,"%[^\n]",cadena);
		fscanf(in,"%c",&car);
		if(strcmp02(cadena,cadena_ant))
			parsea_cadena_de_items(cadena);
		strcpy(cadena_ant,cadena);
		}
	fclose(in);
	}
else
	{
	printf("No se encontró el archivo %s\n",entrada);
	ban=0;
	}
return ban;
}

//Esta funcion lee un long double sencillo
long double encuentra_double(int &pos,char cad[])
{
char car;
long double num=0,fraccion=1;
int ban=0;
//Posiciona la cadena en el primer digito
for(;!ban;++pos)
	if(cad[pos]>='0' && cad[pos]<='9')
		{
		ban=1;
		break;
		}
//Lee la parte entera del numero
for(;ban && cad[pos];++pos)
	{
	if(cad[pos]>='0' && cad[pos]<='9')
		num=num*10+cad[pos]-'0';
	else
		{
		ban=0;
		break;
		}
	}
if(cad[pos]=='.')
	{
	++pos;
	car=cad[pos];
	while(car>='0' && car<='9') // convierte la cadena a número fraccionario
		{
		fraccion/=10;
		num=num+(car-'0')*fraccion;
		++pos;
		car=cad[pos];
		}
	}
return num;
}

int parsea_cadena_de_defectos(char cad[])
{
int pos=0,ban=0,p1;
//Lee el id del defecto
defecto[num_defectos].id=encuentra_entero(pos,cad);
//Lee el # de plate
defecto[num_defectos].plate=encuentra_entero(pos,cad);
//Lee la coordenada X donde empieza el defecto
defecto[num_defectos].x=encuentra_double(pos,cad);
//Lee la coordenada Y donde empieza el defecto
defecto[num_defectos].y=encuentra_double(pos,cad);
//Lee el ancho del defecto
defecto[num_defectos].witdth=encuentra_double(pos,cad);
//Lee la altura del defecto
defecto[num_defectos].heigh=encuentra_double(pos,cad);
//Inserta el elemento en su lista
p1=defecto[num_defectos].plate;
inserta(L_DEFECT[p1],num_defectos);
++num_defectos;
return ban;
}

int lee_defects(char archivo[])
{
FILE *in;
char entrada[80],cadena[120],car,cadena_ant[120]="";
int i,ban=1;
strcpy(entrada,archivo);
strcat(entrada,"_defects.csv");
in=fopen(entrada,"rb+");
if(in!=NULL)
	{
	//Inicializa las listas de los defectos (por cada plate)
	for(i=0;i<MAX_DEFECTS;++i)
		{
		L_DEFECT[i].num=0;
		L_DEFECT[i].next=NULL;
		}
	//Lee la primera linea de encabezados
	fscanf(in,"%[^\n]",cadena);
	fscanf(in,"%c",&car);
	while(!feof(in))
		{
		//Lee una linea de datos
		fscanf(in,"%[^\n]",cadena);
		fscanf(in,"%c",&car);
		if(strcmp02(cadena,cadena_ant))
			parsea_cadena_de_defectos(cadena);
		strcpy(cadena_ant,cadena);
		}	
	fclose(in);
	}
else
	{
	printf("No se encontró el archivo %s\n",entrada);
	ban=0;
	}
return ban;
}

void inicializa_variables()
{
static int primero=1;
int i,ban=1;
//Inicializa la bandera y lo del backtraking
for(i=0;i<num_items;++i)
	{
	bandera[i]=0;
	backtraking[i].pos_it1=-1;
	backtraking[i].pos_sol=-1;
	if(primero)
		area_items+=item[i].length*item[i].witdth;
	}
backtraking[0].x_min=0;
backtraking[0].y_min=0;
backtraking[0].x_max=0;
backtraking[0].y_max=0;
backtraking[0].x_act=0;
backtraking[0].plate=0;
//Cuenta las pilas (conjunto de ordenes)
for(i=0;i<MAX_ITEMS && ban;++i)
	if(L_STACK[i].num)
		num_stacks=i+1;
//Cuenta los plates (el # de hojas de vidrio grandes que se cortaran)
ban=1;
for(i=0;i<num_defectos && ban;++i)
	if(L_DEFECT[i].num)
		++num_plates;
	else
		ban=0;
if(primero)
	max_costo=mejor_costo=100*WIDTH_PLATE*HEIGH_PLATE;
primero=0;
}

//Esta funcion calcula los items posibles (candidatos) que pueden ser programados para ser cortados
int calcula_candidatos()
{
vertice *p;
int i,cont=0,ban;
for(i=0;i<num_stacks;++i)
	{
	p=L_STACK[i].next;
	ban=0;
	while(p!=NULL && !ban)
		{
		if(!bandera[p->id])
			{
			candidato[cont]=p->id;
			++cont;
			ban=1;
			}
		p=p->next;
		}
	}
return cont;
}

//Esta funcion calcula los items posibles (candidatos) que pueden ser programados para ser cortados, ordenandolos por su lado corto (mas largo primero)
int calcula_candidatos_lado_corto_mas_largo()
{
vertice *p;
int i,j,cont=0,ban,primero=1,lado_corto=-1,l_corto01,it1;
for(i=0;i<num_stacks;++i)
	{
	p=L_STACK[i].next;
	ban=0;
	while(p!=NULL && !ban)
		{
		if(!bandera[p->id])
			{
			ban=1;
			if(item[p->id].length>item[p->id].witdth)
				lado_corto=item[p->id].witdth;
			else
				lado_corto=item[p->id].length;
			j=cont;//DEB
			if(j>0)
				{
				it1=candidato[j-1];
				if(item[it1].length>item[it1].witdth)
					l_corto01=item[it1].witdth;
				else
					l_corto01=item[it1].length;
				}
			while(j>0 && l_corto01<lado_corto)
				{
				candidato[j]=candidato[j-1];
				--j;
				if(j>0)
					{
					it1=candidato[j-1];
					if(item[it1].length>item[it1].witdth)
						l_corto01=item[it1].witdth;
					else
						l_corto01=item[it1].length;
					}
				}
			candidato[j]=p->id;
			++cont;
			}
		p=p->next;
		}
	}
return cont;
}

//Esta funcion calcula los items posibles (candidatos) que pueden ser programados para ser cortados, ordenandolos por su lado largo (mas largo primero)
int calcula_candidatos_lado_mas_largo()
{
vertice *p;
int i,j,cont=0,ban,primero=1,lado_largo=-1,l_largo01,it1;
for(i=0;i<num_stacks;++i)
	{
	p=L_STACK[i].next;
	ban=0;
	while(p!=NULL && !ban)
		{
		if(!bandera[p->id])
			{
			ban=1;
			if(item[p->id].length<item[p->id].witdth)
				lado_largo=item[p->id].witdth;
			else
				lado_largo=item[p->id].length;
			j=cont;//DEB
			if(j>0)
				{
				it1=candidato[j-1];
				if(item[it1].length<item[it1].witdth)
					l_largo01=item[it1].witdth;
				else
					l_largo01=item[it1].length;
				}
			while(j>0 && l_largo01<lado_largo)
				{
				candidato[j]=candidato[j-1];
				--j;
				if(j>0)
					{
					it1=candidato[j-1];
					if(item[it1].length<item[it1].witdth)
						l_largo01=item[it1].witdth;
					else
						l_largo01=item[it1].length;
					}
				}
			candidato[j]=p->id;
			++cont;
			}
		p=p->next;
		}
	}
return cont;
}

//Esta funcion calcula los items posibles (candidatos) que pueden ser programados para ser cortados, ordenandolos por su area (area mas grande primero)
int calcula_candidatos_mayor_area()
{
vertice *p;
int i,j,cont=0,ban,primero=1,area_item=-1,area01,it1;
for(i=0;i<num_stacks;++i)
	{
	p=L_STACK[i].next;
	ban=0;
	while(p!=NULL && !ban)
		{
		if(!bandera[p->id])
			{
			ban=1;
			area_item=item[p->id].witdth*item[p->id].length;
			j=cont;//DEB
			if(j>0)
				{
				it1=candidato[j-1];
				area01=item[it1].witdth*item[it1].length;
				}
			while(j>0 && area01<area_item)
				{
				candidato[j]=candidato[j-1];
				--j;
				if(j>0)
					{
					it1=candidato[j-1];
					area01=item[it1].witdth*item[it1].length;
					}
				}
			candidato[j]=p->id;
			++cont;
			}
		p=p->next;
		}
	}
return cont;
}


int encuentra_defectos(int it1,int p1,int x_ini,int y_ini,int tipo)
{
nodo *p;
int cont=0,x_min,y_min,x_max,y_max,x1,x2,y1,y2;
p=L_DEFECT[p1].next;
x_min=x_ini;
y_min=y_ini;
if(tipo)
	{
	x_max=x_ini+item[it1].length;
	y_max=y_ini+item[it1].witdth;
	}
else
	{
	x_max=x_ini+item[it1].witdth;
	y_max=y_ini+item[it1].length;
	}
while(p!=NULL)
	{
	x1=defecto[p->num].x;
	x2=x1+defecto[p->num].witdth;
	y1=defecto[p->num].y;
	y2=y1+defecto[p->num].heigh;
	if((x1>x_min && x1<x_max) || (x2>x_min && x2<x_max))
		if((y1>y_min && y1<y_max) || (y2>y_min && y2<y_max))
			{
			array_defect[cont]=p->num;
			++cont;
			defecto_encontrado[num_defectos_encontrados++]=p->num;
			}
	p=p->next;
	}
return cont;
}

int defecto_a_la_derecha(int num_defects_area)
{
int i,d_max,x_max,d1,x1;
d_max=array_defect[0];
x_max=defecto[d_max].x+defecto[d_max].witdth;
for(i=1;i<num_defects_area;++i)
	{
	d1=array_defect[i];
	x1=defecto[d1].x+defecto[d1].witdth;
	if(x1>x_max)
		{
		x_max=x1;
		d_max=d1;
		}
	}
return d_max;
}

int encuentra_defecto_minima_y(int num_defectos_encontrados)
{
int i,d_min,d1,y_min,y_act;
d1=defecto_encontrado[0];
y_min=defecto[d1].y+defecto[d1].heigh;
d_min=d1;
for(i=1;i<num_defectos_encontrados;++i)
	{
	d1=defecto_encontrado[i];
	y_act=defecto[d1].y+defecto[d1].heigh;
	if(y_act<y_min)
		{
		y_min=y_act;
		d_min=d1;
		}
	}
return d_min;
}

//Esta funcion recibe x_act, que es la primera coordenada x de donde puede ser ubicado el item
int encuentra_pos(int x_act,int y_ini,int it1,int p1,int &x_res,int &d_max,int tipo,int x_min)
{
int ban=0,cont,cota_y,cota_x,d1=-1,x_ant;
x_ant=x_act;
d_max=-1;
num_defectos_encontrados=0;
if(tipo) //Cuando tipo vale 1 el item esta transpuesto
	{
	cota_y=item[it1].witdth;
	cota_x=item[it1].length;
	}
else //Cuando tipo vale 0 el item esta en su posicion original
	{
	cota_y=item[it1].length;
	cota_x=item[it1].witdth;
	}

while(!ban)
	{
	if(y_ini+cota_y>HEIGH_PLATE-MIN_WASTE && y_ini+cota_y!=HEIGH_PLATE)
		{
		ban=0;
//		printf("ERROR: No se encontro la posicion para un item :(\n");
		break;
		}	
	if(x_act+cota_x>WIDTH_PLATE-MIN_WASTE && x_act+cota_x!=WIDTH_PLATE)
		{
		ban=0;
	//	printf("ERROR: No se encontro la posicion para un item :(\n");
		break;
		}
	if((x_act+cota_x)-x_min>MAX_CORTE_TIPO_UNO) //El primer corte debe ser tipo uno
		{
		ban=0;
		//printf("ERROR: No se encontro la posicion para un item :(\n");
		break;
		}
	cont=encuentra_defectos(it1,p1,x_act,y_ini,tipo);
	if(cont)
		{
		d1=defecto_a_la_derecha(cont);
		x_act=defecto[d1].x+defecto[d1].witdth;
		//defecto_encontrado[num_defectos_encontrados]=d1;
		//++num_defectos_encontrados;
		if(x_act-x_ant<MIN_WASTE)
			x_act=x_ant+MIN_WASTE;
		}
	else
		ban=1;
	}
if(ban)
	{
	x_res=x_act;
	d_max=d1;
	}
else
	if(num_defectos_encontrados)
		{
		x_res=x_act;
		d_max=encuentra_defecto_minima_y(num_defectos_encontrados);
		}
return ban;
}

int calcula_posiciones_primer_item(int it1,int p1,int x_ini,int y_ini,int cont_ini)
{
int cont=0,ban_ori=0,ban_transpuesto=0,x_act=0,y_act=0,x_res,d1,ban,y1;
x_act=x_ini;
y_act=y_ini;
cont=cont_ini;
//Busca las posiciciones tomando en cuenta que el objeto estara en su posicion original
while(!ban_ori)
	{
	ban=encuentra_pos(x_act,y_act,it1,p1,x_res,d1,0,x_ini);
	if(ban)
		{
		posible_lugar[cont].x=x_res;
		posible_lugar[cont].y=y_act;
		posible_lugar[cont].tipo=0;
		posible_lugar[cont].plate=p1;
		posible_lugar[cont].id=it1;
		posible_lugar[cont].tipo_new_posicion=ANT_POSICION_NULA;
		if(d1!=-1)
			{
			y1=defecto[d1].y+defecto[d1].heigh;
			y_act=y1;
			if(y_act-y_ini<MIN_WASTE)
				y_act=y_ini+MIN_WASTE;
			}
		if(x_res==x_ini)
			ban_ori=1;
		++cont;
		}
	else
		if(d1!=-1)
			{
			y1=defecto[d1].y+defecto[d1].heigh;
			y_act=y1;
			if(y_act-y_ini<MIN_WASTE)
				y_act=y_ini+MIN_WASTE;
			}
		else
			break;
	}
//Busca las posiciciones tomando en cuenta que el objeto estara transpuesto 90 grados
y_act=y_ini;
if(item[it1].length==item[it1].witdth)
	ban_transpuesto=1;
while(!ban_transpuesto)
	{
	ban=encuentra_pos(x_act,y_act,it1,p1,x_res,d1,1,x_ini);
	if(ban)
		{
		posible_lugar[cont].x=x_res;
		posible_lugar[cont].y=y_act;
		posible_lugar[cont].tipo=1;
		posible_lugar[cont].plate=p1;
		posible_lugar[cont].id=it1;
		posible_lugar[cont].tipo_new_posicion=ANT_POSICION_NULA;
		if(d1!=-1)
			{
			y1=defecto[d1].y+defecto[d1].heigh;
			y_act=y1;
			if(y_act-y_ini<MIN_WASTE)
				y_act=y_ini+MIN_WASTE;
			}
		if(x_res==x_ini)
			ban_transpuesto=1;
		++cont;
		}
	else
		if(d1!=-1)
			{
			y1=defecto[d1].y+defecto[d1].heigh;
			y_act=y1;
			if(y_act-y_ini<MIN_WASTE)
				y_act=y_ini+MIN_WASTE;
			}
		else
			break;
	}
//printf("Encontro %d movimientos iniciales\n",cont);
return (cont-cont_ini);
}

//Esta funcion recibe  parametros, el indice del item, el indice del plate, el indice de la posicion calculada y 
//el indice que le corresponde en la solucion actual
int asigna_posicion(int it1,int p1,int pos_posicion,int pos_solucion,int &y_act)
{
int x;
solution_item[pos_solucion]=posible_lugar[pos_posicion];
x=solution_item[pos_solucion].x;
y_act=solution_item[pos_solucion].y;
if(solution_item[pos_solucion].tipo==0)
	{
	x+=item[it1].witdth;
	y_act+=item[it1].length;
	}
else
	{
	x+=item[it1].length;
	y_act+=item[it1].witdth;
	}
return x;
}


//Esta funcion, cuando se esta contruyendo la solucion aleatoria, encuentra la diferencia minima que debe existir para poder poner
//un item arriba de la secuencia actual 
int encuentra_cota_corte_dos(int pos_max,int y_max)
{
int dif,ini,fin,ban=1,p1,p2,y1,y2,it1,i,tipo,cota=0;
//Primero encuentra el intervalo
fin=pos_max-1;
p1=solution_item[fin].plate;
y1=solution_item[fin].y;
for(ini=pos_max-1;ban;--ini)
	{
	p2=solution_item[ini].plate;
	if(p1!=p2)
		{
		ban=0;
		++ini;
		break;
		}
	y2=solution_item[ini].y;
	if(y1!=y2)
		{
		ban=0;
		++ini;
		break;
		}
	tipo=solution_item[ini].tipo_new_posicion;
	if(tipo==ANT_POSICION_NULA || tipo==POSICION_ARRIBA || tipo==POSICION_NUEVA)
		{
		ban=0;
		break;
		}
	}
//Luego trabaja con el intervalo
for(i=ini;i<=fin;++i)
	{
	it1=solution_item[i].id;
	y1=solution_item[i].y;
	if(solution_item[i].tipo==0)
		y1+=item[it1].length;
	else
		y1+=item[it1].witdth;
	dif=y_max-y1;
	if(dif && dif<MIN_WASTE)
		cota=MIN_WASTE;
	}
return cota;
}

//Esta funcion, cuando se esta contruyendo la solucion aleatoria, encuentra la diferencia minima que debe existir para poder poner
//un item a la derecha de la secuencia actual , Esta funcion podría fallar si exite un item con cortes delgados
//Entonces supones que no habra un pedido con altura o anchura menor que el desperdicio minimo
int encuentra_cota_corte_uno(int pos_max,int x_max)
{
int dif,ini,fin,p1,p2,x1,ban=1,tipo,i,it1,cota=0,tipo_posicion;
//Primero encuentra el intervalo
fin=pos_max-1;
p1=solution_item[fin].plate;
for(ini=pos_max-1;ban;--ini)
	{
	p2=solution_item[ini].plate;
	if(p1!=p2)
		{
		ban=0;
		++ini;
		break;
		}	
	tipo=solution_item[ini].tipo_new_posicion;
	if(tipo==ANT_POSICION_NULA || tipo==POSICION_NUEVA)
		{
		ban=0;
		break;
		}
	}
//Luego trabaja con el intervalo
for(i=ini;i<=fin;++i)
	{
	it1=solution_item[i].id;
	x1=solution_item[i].x;
	if(solution_item[i].tipo==0) //La posicion es original
		x1+=item[it1].witdth;
	else //La posicion esta transpuesta
		x1+=item[it1].length;
	if(i+1<=fin)
		tipo_posicion=solution_item[i+1].tipo_new_posicion;
	else
		tipo_posicion=-1; 
	if(i+1>fin || tipo_posicion==POSICION_ARRIBA)
		{
		dif=x_max-x1;
		if(dif && dif<MIN_WASTE)
			cota=MIN_WASTE;
		}
	}
return cota;
}

int calcula_posiciones_siguiente_item(int it1,int p1,int x_min,int x_act,int y_min,int y_max,int x_max,int pos_max)
{
int cont=0,ban,x_res,d1,ban_arriba=0,ban_transpuesto=0,cont_aux,i,dif_min,ban_ori=0,y_act,y1,ban_corte_uno=1;
//Busca la primera posicion a la derecha tomando en cuenta que el objeto estara en su posicion original
ban=encuentra_pos(x_act,y_min,it1,p1,x_res,d1,0,x_min);
if(ban)
	{
	posible_lugar[cont].x=x_res;
	posible_lugar[cont].y=y_min;
	posible_lugar[cont].tipo=0;
	posible_lugar[cont].plate=p1;
	posible_lugar[cont].id=it1;
	posible_lugar[cont].tipo_new_posicion=POSICION_DER;
	arreglo_reco_x[cont]=x_res-x_act;
	arreglo_reco_y[cont]=0;
	++cont;
	if(x_res+item[it1].witdth<=x_max)
		ban_corte_uno=0;
	}
//Busca la primera posicion a la derecha tomando en cuenta que el objeto estara transpuesto 90 grados 
if(item[it1].length!=item[it1].witdth)
	{
	ban=encuentra_pos(x_act,y_min,it1,p1,x_res,d1,1,x_min);
	if(ban)
		{
		posible_lugar[cont].x=x_res;
		posible_lugar[cont].y=y_min;
		posible_lugar[cont].tipo=1;
		posible_lugar[cont].plate=p1;
		posible_lugar[cont].id=it1;
		posible_lugar[cont].tipo_new_posicion=POSICION_DER;
		arreglo_reco_x[cont]=x_res-x_act;
		arreglo_reco_y[cont]=0;
		++cont;
		if(x_res+item[it1].length<=x_max)
			ban_corte_uno=0;
		}
	}
//Busca la primera posicion ARRIBA DE LOS ITEMS ACTUALES tomando en cuenta que el objeto estara en su posicion original
dif_min=encuentra_cota_corte_dos(pos_max,y_max);
y_act=y_max+dif_min;
//Busca las posiciciones tomando en cuenta que el objeto estara en su posicion original
while(!ban_ori)
	{
	ban=encuentra_pos(x_min,y_act,it1,p1,x_res,d1,0,x_min);
	if(ban)
		{
		ban_arriba=1;
		posible_lugar[cont].x=x_res;
		posible_lugar[cont].y=y_act;
		posible_lugar[cont].tipo=0;
		posible_lugar[cont].plate=p1;
		posible_lugar[cont].id=it1;
		posible_lugar[cont].tipo_new_posicion=POSICION_ARRIBA;
		arreglo_reco_x[cont]=x_res-x_min;
		arreglo_reco_y[cont]=y_act-y_max-dif_min;
		if(d1!=-1)
			{
			y1=defecto[d1].y+defecto[d1].heigh;
			y_act=y1;
			if(y_act-y_max<MIN_WASTE)
				y_act=y_max+MIN_WASTE;
			}
		if(x_res==x_min)
			ban_ori=1;
		++cont;
		if(x_res+item[it1].witdth<=x_max)
			ban_corte_uno=0;
		}
	else
		if(d1!=-1)
			{
			y1=defecto[d1].y+defecto[d1].heigh;
			y_act=y1;
			if(y_act-y_max<MIN_WASTE)
				y_act=y_max+MIN_WASTE;
			}
		else
			break;
	}
//Busca las posiciciones tomando en cuenta que el objeto estara transpuesto 90 grados
y_act=y_max+dif_min;
if(item[it1].length==item[it1].witdth)
	ban_transpuesto=1;
while(!ban_transpuesto)
	{
	ban=encuentra_pos(x_min,y_act,it1,p1,x_res,d1,1,x_min);
	if(ban)
		{
		ban_arriba=1;
		posible_lugar[cont].x=x_res;
		posible_lugar[cont].y=y_act;
		posible_lugar[cont].tipo=1;
		posible_lugar[cont].plate=p1;
		posible_lugar[cont].id=it1;
		posible_lugar[cont].tipo_new_posicion=POSICION_ARRIBA;
		arreglo_reco_x[cont]=x_res-x_min;
		arreglo_reco_y[cont]=y_act-y_max-dif_min;
		if(d1!=-1)
			{
			y1=defecto[d1].y+defecto[d1].heigh;
			y_act=y1;
			if(y_act-y_max<MIN_WASTE)
				y_act=y_max+MIN_WASTE;
			}
		if(x_res==x_min)
			ban_transpuesto=1;
		++cont;
		if(x_res+item[it1].length<=x_max)
			ban_corte_uno=0;
		}
	else
		if(d1!=-1)
			{
			y1=defecto[d1].y+defecto[d1].heigh;
			y_act=y1;
			if(y_act-y_max<MIN_WASTE)
				y_act=y_max+MIN_WASTE;
			}
		else
			break;
	}
//Busca la primera posicion en UN NUEVO CORTE DE UNO
//if(!ban_arriba) //Se comentario esto para que funcionara mejor
if(ban_corte_uno)
	{
	dif_min=encuentra_cota_corte_uno(pos_max,x_max);
	cont_aux=calcula_posiciones_primer_item(it1,p1,x_max+dif_min,0,cont);
	if(cont_aux)
		{
		for(i=0;i<cont_aux;++i)
			{
			posible_lugar[cont+i].tipo_new_posicion=POSICION_NUEVA;
			arreglo_reco_x[cont+i]=posible_lugar[cont+i].x-(x_max+dif_min);
			arreglo_reco_y[cont+i]=posible_lugar[cont+i].y;
			}
		cont+=cont_aux;
		} 
	}
return cont;
}

long int calcula_costo()
{
long int a1;
int i,p1=-1,plates_used=0,x_max=0,it1,x_act,dif,residuo=0;
for(i=0;i<num_items;++i)
	{
	x_act=solution_item[i].x;
	it1=solution_item[i].id;
	if(solution_item[i].tipo==0) //Esta en su posicion original
		x_act+=item[it1].witdth;
	else
		x_act+=item[it1].length;
	if(p1!=solution_item[i].plate)
		{
		++plates_used;
		p1=solution_item[i].plate;
		x_max=x_act;
		}
	if(x_act>x_max)
		x_max=x_act;
	}
a1=plates_used*WIDTH_PLATE*HEIGH_PLATE;
a1-=area_items;
dif=WIDTH_PLATE-x_max;
if(dif>=MIN_CORTE_TIPO_UNO)
	residuo=dif*HEIGH_PLATE;
a1-=residuo;
return a1;
}

long int calcula_costo_parcial(int pos)
{
long int a1,area_items_parcial=0,residuo=0;
int i,p1=-1,plates_used=0,x_max=0,it1,x_act,dif;
for(i=0;i<pos;++i)
	{
	x_act=solution_item[i].x;
	it1=solution_item[i].id;
	if(solution_item[i].tipo==0) //Esta en su posicion original
		x_act+=item[it1].witdth;
	else
		x_act+=item[it1].length;
	if(p1!=solution_item[i].plate)
		{
		++plates_used;
		p1=solution_item[i].plate;
		x_max=x_act;
		}
	if(x_act>x_max)
		x_max=x_act;
	area_items_parcial+=item[it1].length*item[it1].witdth;
	}
a1=plates_used*WIDTH_PLATE*HEIGH_PLATE;
a1-=area_items_parcial;
dif=WIDTH_PLATE-x_max;
if(dif>=MIN_CORTE_TIPO_UNO)
	residuo=dif*HEIGH_PLATE;
a1-=residuo;
return a1;
}

//Esta funcion calcula el desperdicio de un corte uno, sin embargo, es posible mejorarlo si se tuviera hasta donde llego el otro corte uno
long int calcula_costo_corte_uno(int pos_ini,int pos)
{
long int a1,area_items_uno=0;
int i,p1=-1,plates_used=0,x_max=0,it1,x_act,residuo=0,x_min;
x_min=solution_item[pos_ini].x;
for(i=pos_ini;i<pos;++i)
	{
	x_act=solution_item[i].x;
	it1=solution_item[i].id;
	if(x_act<x_min)
		x_min=x_act;
	if(solution_item[i].tipo==0) //Esta en su posicion original
		x_act+=item[it1].witdth;
	else
		x_act+=item[it1].length;
	if(x_act>x_max)
		x_max=x_act;
	area_items_uno+=item[it1].length*item[it1].witdth;
	}
if(pos_ini==0)//Este codigo es por si el primer item es uno y existe un defecto que lo recorra
	x_min=0;
else
	if(pos_ini && solution_item[pos_ini-1].plate!=solution_item[pos_ini].plate)
		x_min=0;
a1=(x_max-x_min)*HEIGH_PLATE;
a1-=area_items_uno;
return a1;
}

int calcula_posicion_info_corte_uno(int pos,int n)
{
int i,ban=0,pos_info_corte_uno=-1;
for(i=0;i<=n && !ban;++i)
	if(pos>=info_corte_uno[i].pos_ini && pos<=info_corte_uno[i].pos_fin)
		{
		ban=1;
		pos_info_corte_uno=i;
		}
if(pos_info_corte_uno==-1)
	pos_info_corte_uno=n;
return pos_info_corte_uno;
}

int calcula_plates_usados()
{
int i,p1=-1,cont=0;
for(i=0;i<num_items;++i)
	if(p1!=solution_item[i].plate)
		{
		++cont;
		p1=solution_item[i].plate;
		}
return cont;
}

int calcula_inicio_fin_items_plate(int &ini,int &fin,int p1)
{
int i,ban=0;
//busca el inicio
for(i=ini;!ban && i<num_items;++i)
	if(solution_item[i].plate==p1)
		{
		ban=1;
		ini=fin=i;
		}
//busca el fin
for(;ban && i<num_items;++i)
	if(solution_item[i].plate!=p1)
		{
		fin=i;
		ban=0;
		}
if(ban)
	fin=num_items;
return 1;
}

int siguiente_corte_uno(int ini,int fin,int x_ini,int &x_fin,int &type,int &new_ini)
{
int i,ban=0,x_max=0,it1,x_act,cont=0,it_posible=-1,x_item_min=WIDTH_PLATE+1,dif_min;
int width,length;
for(i=ini;i<fin;++i)
	{
	x_act=solution_item[i].x;
	it1=solution_item[i].id;
	if(solution_item[i].tipo==0)
		x_act+=item[it1].witdth;
	else
		x_act+=item[it1].length;
	if(solution_item[i].tipo_new_posicion==POSICION_NUEVA && i!=ini)
		{
		ban=1;
		break;
		}	
	else
		if(x_act>=x_ini)
			{
			ban=1;
			it_posible=it1;
			++cont;
			if(x_act>x_max)
				x_max=x_act;
			if(solution_item[i].x<x_item_min)
				x_item_min=solution_item[i].x;
			}
	}
new_ini=i; //Se avanza con la posicion inicial
if(ban)
	x_fin=x_max;
else
	x_max=x_fin=WIDTH_PLATE;
if(cont>1)
	{
	if(x_item_min>x_ini)
		{//Hay que hacer varios ajustes
		type=-1;
		x_fin=x_item_min;
		width=x_fin-x_ini;
		new_ini=ini;
		}
	else
		{
		type=-2;
		dif_min=encuentra_cota_corte_uno(new_ini,x_max);
		if(dif_min)
				x_fin=x_max+dif_min; //solution_item[new_ini].x;
		width=x_fin-x_ini;
		if(WIDTH_PLATE-x_fin<MIN_WASTE)
			x_max=x_fin=WIDTH_PLATE;
		}
	}
else
	if(cont==1)
		{
		it1=it_posible;
		if(new_ini!=fin)
			x_fin=solution_item[new_ini].x;
		width=x_fin-x_ini;
		length=HEIGH_PLATE;
		if(x_item_min>x_ini)
			{//Hay que hacer varios ajustes
			type=-1;
			x_fin=x_item_min;
			width=x_fin-x_ini;
			new_ini=ini;
			}
		else
			if((item[it1].witdth==width && item[it1].length==length)  || (item[it1].witdth==length && item[it1].length==width))
				type=it1;
			else
				type=-2;

		}
	else
		if(x_max-x_ini>=MIN_CORTE_TIPO_UNO && x_max==WIDTH_PLATE)
			type=-3;
		else
			type=-1;
if(x_max==WIDTH_PLATE)
	ban=0;
else
	ban=1;
return ban;
}

int calcula_inicio_fin_items_corte_uno(int &ini,int &fin,int p1)
{
int i,ban=0;
//busca el inicio
for(i=ini;!ban && i<num_items;++i)
	if(solution_item[i].plate==p1)
		{
		ban=1;
		ini=fin=i;
		}
//busca el fin
for(;ban && i<num_items;++i)
	if(solution_item[i].plate!=p1 || solution_item[i].tipo_new_posicion==POSICION_NUEVA)
		{
		fin=i;
		ban=0;
		}
if(ban)
	fin=num_items;
return 1;
}

int siguiente_corte_dos(int ini,int fin,int y_ini,int &y_fin,int &type,int &new_ini,int x1,int x2)
{
int i,y_act,y_max=0,it1,ban=0,it_posible,cont=0,y_item_min=HEIGH_PLATE+1;
int width,length;
for(i=ini;i<fin;++i)
	{
	y_act=solution_item[i].y;
	it1=solution_item[i].id;
	if(solution_item[i].tipo==0)
		y_act+=item[it1].length;
	else
		y_act+=item[it1].witdth;
	if((solution_item[i].tipo_new_posicion==POSICION_NUEVA && i!=ini) || (solution_item[i].tipo_new_posicion==POSICION_ARRIBA && i!=ini))
		{
		ban=1;
		break;
		}	
	else
		if(y_act>=y_ini)
			{
			ban=1;
			it_posible=it1;
			++cont;
			if(y_act>y_max)
				y_max=y_act;
			if(solution_item[i].y<y_item_min)
				y_item_min=solution_item[i].y;
			}
	}
new_ini=i; //Se avanza con la posicion inicial
if(ban)
	y_fin=y_max;
else
	y_max=y_fin=HEIGH_PLATE;
if(cont>1)
	{
	if(y_item_min>y_ini && y_ini==0)
		{//Hay que hacer varios ajustes
		type=-1;
		y_max=y_fin=y_item_min;
		length=y_fin-y_ini;
		new_ini=ini;
		}
	else
		{
		type=-2;
		if(new_ini!=fin)
			y_fin=solution_item[new_ini].y;
		else
			y_max=y_fin=HEIGH_PLATE;
		length=y_fin-y_ini;
		}
	}
else
	if(cont==1)
		{
		it1=it_posible;
		if(new_ini!=fin)
			y_fin=solution_item[new_ini].y;
		else
			y_max=y_fin=HEIGH_PLATE;
		length=y_fin-y_ini;
		width=x2-x1;
		if(y_item_min>y_ini && y_ini==0)
			{//Hay que hacer varios ajustes
			type=-1;
			y_max=y_fin=y_item_min;
			length=y_fin-y_ini;
			new_ini=ini;
			}
		else
			if((item[it1].witdth==width && item[it1].length==length)  || (item[it1].witdth==length && item[it1].length==width))
				type=it1;
			else
				type=-2;
		}
	else
		type=-1;
if(y_max==HEIGH_PLATE)
	ban=0;
else
	ban=1;
return ban;
}

int calcula_inicio_fin_items_corte_dos(int &ini,int &fin,int p1)
{
int i,ban=0;
//busca el inicio
for(i=ini;!ban && i<num_items;++i)
	if(solution_item[i].plate==p1)
		{
		ban=1;
		ini=fin=i;
		}
//busca el fin
for(;ban && i<num_items;++i)
	if(solution_item[i].plate!=p1 || solution_item[i].tipo_new_posicion==POSICION_NUEVA || solution_item[i].tipo_new_posicion==POSICION_ARRIBA)
		{
		fin=i;
		ban=0;
		}
if(ban)
	fin=num_items;
return 1;
}

//int siguiente_corte_dos(int ini,int fin,int y_ini,int &y_fin,int &type,int &new_ini,int x1,int x2)
int siguiente_corte_tres(int ini,int fin,int x_ini,int &x_fin,int &type,int &new_ini,int x1,int x2,int y_ini,int y_fin)
{
int i,cont=0,x_act,x_max=0,x_min,it1,ban=0;
int width,length;
x_min=x_ini;
for(i=ini;i<fin && cont<1;++i)
	{
	x_act=solution_item[i].x;
	it1=solution_item[i].id;
	if(solution_item[i].tipo==0)
		x_act+=item[it1].witdth;
	else
		x_act+=item[it1].length;
	if(x_act>=x_ini)
		{
		ban=1;
		++cont;
		if(x_act>x_max)
			x_max=x_act;
		x_min=solution_item[i].x;
		}
	}
new_ini=i; //Se avanza con la posicion inicial
if(ban)
	x_fin=x_max;
else
	x_max=x_fin=x2;
if(x_ini!=x_min)
	{//Hay que hacer varios ajustes
	type=-1;
	new_ini=ini;
	x_fin=x_max=x_min;
	}
else
	if(cont==1)
		{
		length=y_fin-y_ini;
		width=x_fin-x_ini;
		if((item[it1].witdth==width && item[it1].length==length)  || (item[it1].witdth==length && item[it1].length==width))
			type=it1;
		else
			type=-2;
		}
	else
		type=-1;
if(x_max==x2)
	ban=0;
else
	ban=1;
return ban;
}

int calcula_cortes_tipo_cuatro(int parent_node,int p1,int ini,int x_ini,int x_fin,int y_ini,int y_fin)
{
int pos,y_new,it1;
pos=parent_node+1;
it1=solution_item[ini].id;
y_new=y_ini;
if(solution_item[ini].tipo==0)
	y_new+=item[it1].length;
else
	y_new+=item[it1].witdth;
impresion_solucion[pos].plate=p1;
impresion_solucion[pos].nodo_id=pos;
impresion_solucion[pos].x=x_ini;
impresion_solucion[pos].y=y_ini;
impresion_solucion[pos].width=x_fin-x_ini;
impresion_solucion[pos].heigh=y_new-y_ini;
impresion_solucion[pos].type=it1; //Es el item
impresion_solucion[pos].cut=4;
impresion_solucion[pos].parent=parent_node;
++pos;
impresion_solucion[pos].plate=p1;
impresion_solucion[pos].nodo_id=pos;
impresion_solucion[pos].x=x_ini;
impresion_solucion[pos].y=y_new;
impresion_solucion[pos].width=x_fin-x_ini;
impresion_solucion[pos].heigh=y_fin-y_new;
impresion_solucion[pos].type=-1; //Es desperdicio
impresion_solucion[pos].cut=4;
impresion_solucion[pos].parent=parent_node;
++pos;
return (pos-parent_node-1);
}

int calcula_cortes_tipo_tres(int parent_node,int p1,int ini,int fin,int x_ini,int x_fin,int y_ini,int y_fin)
{
int pos,x1,x2,type,new_ini,ban;
calcula_inicio_fin_items_corte_dos(ini,fin,p1);
pos=parent_node+1;
x1=x_ini;
do {
	ban=siguiente_corte_tres(ini,fin,x1,x2,type,new_ini,x_ini,x_fin,y_ini,y_fin);
	impresion_solucion[pos].plate=p1;
	impresion_solucion[pos].nodo_id=pos;
	impresion_solucion[pos].x=x1;
	impresion_solucion[pos].y=y_ini;
	impresion_solucion[pos].width=x2-x1;
	impresion_solucion[pos].heigh=y_fin-y_ini;
	impresion_solucion[pos].type=type;
	impresion_solucion[pos].cut=3;
	impresion_solucion[pos].parent=parent_node;
	++pos;
	if(type==-2) 
		pos+=calcula_cortes_tipo_cuatro(pos-1,p1,ini,x1,x2,y_ini,y_fin);
	ini=new_ini;
	x1=x2;
	}
while(ban);
return (pos-parent_node-1);
}

int calcula_cortes_tipo_dos(int parent_node,int p1,int ini,int fin,int x_ini,int x_fin)
{
int pos,ban,y_ini=0,y_fin,new_ini,type;
calcula_inicio_fin_items_corte_uno(ini,fin,p1);
pos=parent_node+1;
do {
	ban=siguiente_corte_dos(ini,fin,y_ini,y_fin,type,new_ini,x_ini,x_fin);
	impresion_solucion[pos].plate=p1;
	impresion_solucion[pos].nodo_id=pos;
	impresion_solucion[pos].x=x_ini;
	impresion_solucion[pos].y=y_ini;
	impresion_solucion[pos].width=x_fin-x_ini;
	impresion_solucion[pos].heigh=y_fin-y_ini;
	impresion_solucion[pos].type=type;
	impresion_solucion[pos].cut=2;
	impresion_solucion[pos].parent=parent_node;
	++pos;
	if(type==-2) 
		pos+=calcula_cortes_tipo_tres(pos-1,p1,ini,new_ini,x_ini,x_fin,y_ini,y_fin);
	ini=new_ini;
	y_ini=y_fin;
	}
while(ban);
return (pos-parent_node-1);
}

int calcula_cortes_tipo_uno(int parent_node,int p1,int ini,int &fin,int ban_ultimo)
{
int x_ini=0,x_fin=0,type,ban,new_ini,pos;
calcula_inicio_fin_items_plate(ini,fin,p1);
pos=parent_node+1;
do {
	ban=siguiente_corte_uno(ini,fin,x_ini,x_fin,type,new_ini);
	impresion_solucion[pos].plate=p1;
	impresion_solucion[pos].nodo_id=pos;
	impresion_solucion[pos].x=x_ini;
	impresion_solucion[pos].y=0;
	impresion_solucion[pos].width=x_fin-x_ini;
	impresion_solucion[pos].heigh=HEIGH_PLATE;
	if(type==-3 && !ban_ultimo) //Este es un ajuste para lo del desperdicio
		type=-1;
	impresion_solucion[pos].type=type;
	impresion_solucion[pos].cut=1;
	impresion_solucion[pos].parent=parent_node;
	++pos;
	if(type==-2) 
		pos+=calcula_cortes_tipo_dos(pos-1,p1,ini,new_ini,x_ini,x_fin);
	ini=new_ini;
	x_ini=x_fin;
	}
while(ban);
return (pos-parent_node-1);
}

int construye_solucion()
{
int num_lineas=0;
int i,num_used_plates,ini=0,fin=0,ban_ultimo=0;
num_used_plates=calcula_plates_usados();
for(i=0;i<num_used_plates;++i)
	{
	impresion_solucion[num_lineas].plate=i;
	impresion_solucion[num_lineas].nodo_id=num_lineas;
	impresion_solucion[num_lineas].x=0;
	impresion_solucion[num_lineas].y=0;
	impresion_solucion[num_lineas].width=WIDTH_PLATE;
	impresion_solucion[num_lineas].heigh=HEIGH_PLATE;
	impresion_solucion[num_lineas].type=-2; //Este item se cortara
	impresion_solucion[num_lineas].cut=0; //Aun no hay corte
	++num_lineas;
	ini=fin;
	if(i==num_used_plates-1)
		ban_ultimo=1;
	num_lineas+=calcula_cortes_tipo_uno(num_lineas-1,i,ini,fin,ban_ultimo);
	}
return num_lineas;
}

void imprime_solucion(char archivo[],int num_lineas,long int costo)
{
FILE *out;
int i;
out=fopen(archivo,"wb+");
if(out!=NULL)
	{
	fprintf(out,"PLATE_ID;NODE_ID;X;Y;WIDTH;HEIGHT;TYPE;CUT;PARENT\r\n");
	for(i=0;i<num_lineas;++i)
		{
		fprintf(out,"%d;%d;%d;%d;",impresion_solucion[i].plate,impresion_solucion[i].nodo_id,impresion_solucion[i].x,impresion_solucion[i].y);
		fprintf(out,"%d;%d;%d;%d;",impresion_solucion[i].width,impresion_solucion[i].heigh,impresion_solucion[i].type,impresion_solucion[i].cut);
		if(impresion_solucion[i].cut)
			fprintf(out,"%d",impresion_solucion[i].parent);
		fprintf(out,"\r\n");
		}
	fclose(out);
	}
else
	printf("No se pudo crear el archivo %s\n",archivo);
}

int encuentra_cota_max(int n)
{
int i,cota;
cota=info_corte_uno[0].desperdicio;
for(i=1;i<n;++i)
	if(info_corte_uno[i].desperdicio>cota)
		cota=info_corte_uno[i].desperdicio;
return cota;
}

//Esta funcion compara entre 2 posibles posiciones del mismo o diferentes items y decide cual utilizar pide que no se rebase x_max en caso
//de que la posicion sea a la derecha o arriba del ultimo item
int compara(int tipo_pos01,int x1,int desplaza01,int tipo_pos02,int x2,int desplaza02,int x_max,int y1,int cota_y)
{
int ban=0;

if(tipo_pos01==ANT_POSICION_NULA)
	{
	if(tipo_pos02==ANT_POSICION_NULA)
		{
		if(desplaza01<desplaza02)
			ban=1; //Gana el 1
		else
			if(desplaza01>desplaza02)
				ban=-1;//Gana el 2
			else
				{
				if(x1>x2)
					ban=-1;
				if(x1<x2)
					ban=1; //Gana el 1
				}
		}
	else//Cualquier posicion es mejor que agarrar una nueva hoja
		ban=-1;
	}
if(tipo_pos01==POSICION_NUEVA)
	{
	if(tipo_pos02==ANT_POSICION_NULA)
		ban=1; //Gana el 1
	else
		if(tipo_pos02==POSICION_NUEVA)
			{
			if(desplaza01<desplaza02)
				ban=1; //Gana el 1
			else
				if(desplaza01>desplaza02)
					ban=-1; //Gana el 2
				else
					{
					if(x1>x2)
						ban=-1;
					if(x1<x2)
						ban=1; //Gana el 1
					}
			}
		else //Puede ser POSICION_DER O POSICION_ARRIBA
			{
			if(x2<=x_max)
				ban=-1; //Gana el x2
			else  
				//if(x1<=x_max)//Gana el x1 (1)
					ban=1;  //Aqui pierde por sobrepasar x_max, pero se puede modificar para que lo sobrepase en máximo un porcentaje o el que lo sobrepase menos
			/*	else
					{
					if(x1>x2)
						ban=-1;
					if(x1<x2)
						ban=1; //Gana el 1
					}*/
			}
	}
if(tipo_pos01==POSICION_DER)
	{
	if(tipo_pos02==ANT_POSICION_NULA)
		ban=1; //Gana el 1
	else
		if(tipo_pos02==POSICION_NUEVA)
			{
/*			if(cota_y!=-1 && y1>cota_y)
				ban=-1; //Gana el 2
			else
				{*/
				if(x1<=x_max)
					ban=1; //Gane el 1
				else
					ban=-1;
			//	}
			}
		else
			if(tipo_pos02==POSICION_DER)
				{
				if(desplaza01<desplaza02)
					ban=1; //Gana el 1
				else
					if(desplaza01>desplaza02)
						ban=-1;
					else
						{
						if(cota_y!=-1 && y1>cota_y)
							ban=-1; //Gana el 2
						else
							{
							if(x1>x2)
								ban=-1; //Gana el 2
							if(x1<x2)
								ban=1; //Gana el 1
							}
						}
				}
			else
				{
				if(x1<=x_max)
					ban=1; //Gana el 1
				else
					ban=-1;
				}
	}
if(tipo_pos01==POSICION_ARRIBA)
	{
	if(tipo_pos02==ANT_POSICION_NULA)
		ban=1; //Gana el 1
	else
		if(tipo_pos02==POSICION_NUEVA)
			{
			if(x1<=x_max)
				ban=1; //Gane el 1
			else
				ban=-1;
			}
		else
			if(tipo_pos02==POSICION_DER)
				{
				if(x2<=x_max)
					ban=-1;
				else
					if(x1<=x_max)
						ban=1;
				}
			else
				if(tipo_pos02==POSICION_ARRIBA)
					{
					if(desplaza01<desplaza02)
						ban=1; //Gana el 1
					else
						if(desplaza01>desplaza02)
							ban=-1; //Gana el 2
						else
							{
							if(x1>x2)
								ban=-1; //Gana el 2
							if(x1<x2)
								ban=1; //Gana el 1
							}
					}
	}
return ban;
}

//Esta funcion es para 2 items diferentes
//Esta funcion compara entre 2 posibles posiciones del mismo o diferentes items y decide cual utilizar pide que no se rebase x_max en caso
//de que la posicion sea a la derecha o arriba del ultimo item
int compara02(int tipo_pos01,int x1,int desplaza01,int tipo_pos02,int x2,int desplaza02,int x_max,int y1,int cota_y)
{
int ban=0;
if(tipo_pos01==ANT_POSICION_NULA)
	{
	if(tipo_pos02==ANT_POSICION_NULA)
		{
		if(desplaza01<desplaza02)
			ban=1; //Gane el 1
		else
			if(desplaza01>desplaza02)
				ban=-1; //Gana el 2
			else
				{
				if(x1<x2)
					ban=-1;
				if(x1>x2)
					ban=1; //Gana el 1
				}
		}
	else//Cualquier posicion es mejor que agarrar una nueva hoja
		ban=-1;
	}
if(tipo_pos01==POSICION_NUEVA)
	{
	if(tipo_pos02==ANT_POSICION_NULA)
		ban=1; //Gana el 1
	else
		if(tipo_pos02==POSICION_NUEVA)
			{
			if(desplaza01<desplaza02)
				ban=1; //Gana el 1
			else
				if(desplaza01>desplaza02)
					ban=-1;
				else
					{
					if(x1<x2)
						ban=-1;
					if(x1>x2)
						ban=1; //Gana el 1
					}
			}
		else //Puede ser POSICION_DER O POSICION_ARRIBA
			{
			if(x2<=x_max)
				ban=-1; //Gana el x2
			else  
				//if(x1<=x_max)//Gana el x1 (1)
					ban=1;  //Aqui pierde por sobrepasar x_max, pero se puede modificar para que lo sobrepase en máximo un porcentaje o el que lo sobrepase menos
			/*	else
					{
					if(x1>x2)
						ban=-1;
					if(x1<x2)
						ban=1; //Gana el 1
					}*/
			}
	}
if(tipo_pos01==POSICION_DER)
	{
	if(tipo_pos02==ANT_POSICION_NULA)
		ban=1; //Gana el 1
	else
		if(tipo_pos02==POSICION_NUEVA)
			{
			if(x1<=x_max)
				ban=1; //Gane el 1
			else
				ban=-1;
			}
		else
			if(tipo_pos02==POSICION_DER)
				{
				if(desplaza01<desplaza02)
					ban=1; //Gana el 1
				else
					if(desplaza01>desplaza02)
						ban=-1;
					else
						{
						if(cota_y!=-1 && y1>cota_y)
							ban=-1; //Gana el 2
						else
							{
							if(x1<=x_max && x2>x_max)
								ban=1;//Gana el 1
							else
								if(x1>x_max && x2<=x_max)
									ban=-1;//Gana el 2
								else
									{
									if(x1>x2)
										ban=1; //Gana el 1
									if(x1<x2)
										ban=-1; //Gana el 2
									}
							}
						}
				}
			else
				{
				if(x1<=x_max)
					ban=1; //Gana el 1
				else
					ban=-1;
				}
	}
if(tipo_pos01==POSICION_ARRIBA)
	{
	if(tipo_pos02==ANT_POSICION_NULA)
		ban=1; //Gana el 1
	else
		if(tipo_pos02==POSICION_NUEVA)
			{
			if(x1<=x_max)
				ban=1; //Gane el 1
			else
				ban=-1;
			}
		else
			if(tipo_pos02==POSICION_DER)
				{
				if(x2<=x_max)
					ban=-1;
				else
					if(x1<=x_max)
						ban=1;
				}
			else
				if(tipo_pos02==POSICION_ARRIBA)
					{
					if(desplaza01<desplaza02)
						ban=1; //Gana el 1
					else
						if(desplaza01>desplaza02)
							ban=-1; //Gana el 2
						else
							{
							if(x1<=x_max && x2>x_max)
								ban=1;//Gana el 1
							else
								if(x1>x_max && x2<=x_max)
									ban=-1;//Gana el 2
								else
									{
									if(x1>x2)
										ban=1; //Gana el 1
									if(x1<x2)
										ban=-1; //Gana el 2
									}
							}
					}
	}
return ban;
}

int encuentra_mejor_movimiento01(int cont_items,int &mejor_candidato,int pos,int p1,int x_min,int y_min,int x_max,int y_max,int x_act,double factor,int ban_forzoso)
{
int mejor_pos_sol=-1,pos_sol,cota_y=-1,ban_compara;
int i,j,ban=0,it1,cont,tipo,tipo_posicion,mejor_tipo_posicion,mejor_x_posicion;
int x1,y1,min_x_posicion,min_reco_x,min_reco_y,mejor_reco_x,mejor_reco_y,min_tipo_posicion,dif_y;
for(i=0;i<cont_items && !ban;++i) //Elige el mejor candidato con su "mejor" posicion segun un criterio gloton
	{
	it1=candidato[i];
	if(pos==0) //Si es el primer elemento de una hoja
		{
		cont=calcula_posiciones_primer_item(it1,p1,0,0,0);
		for(j=0;j<cont;++j)
			{
			arreglo_reco_x[j]=posible_lugar[j].x;
			arreglo_reco_y[j]=posible_lugar[j].y;
			}
		if(!cont)
			continue;
		tipo=posible_lugar[0].tipo;
		x1=posible_lugar[0].x;
		y1=posible_lugar[0].y;
		if(tipo) //Si vale 1 está transpuesto
			{
			x1+=item[it1].length;
			y1+=item[it1].witdth;
			}
		else//Si vale 0 esta en su posicion original
			{
			x1+=item[it1].witdth;
			y1+=item[it1].length;
			}
		pos_sol=0;
		min_x_posicion=x1;
		min_reco_x=arreglo_reco_x[0];
		min_reco_y=arreglo_reco_y[0];
		for(j=1;j<cont;++j) //Busca la posicion que incremente menos la x
			{
			tipo=posible_lugar[j].tipo;
			x1=posible_lugar[j].x;
			y1=posible_lugar[j].y;
			if(tipo) //Si vale 1 está transpuesto
				{
				x1+=item[it1].length;
				y1+=item[it1].witdth;
				}
			else//Si vale 0 esta en su posicion original
				{
				x1+=item[it1].witdth;
				y1+=item[it1].length;
				}
			if(arreglo_reco_x[j]+arreglo_reco_y[j]<min_reco_x+min_reco_y)
				{
				min_reco_x=arreglo_reco_x[j];
				min_reco_y=arreglo_reco_y[j];
				min_x_posicion=x1;
				pos_sol=j;
				}
			else
				if(arreglo_reco_x[j]+arreglo_reco_y[j]==min_reco_x+min_reco_y)
					if(x1<min_x_posicion)
						{
						min_reco_x=arreglo_reco_x[j];
						min_reco_y=arreglo_reco_y[j];
						min_x_posicion=x1;
						pos_sol=j;
						}
			}
			//ban=1; //Ya se eligió la mejor posicion :)
		if(mejor_pos_sol==-1)
			{
			mejor_pos_sol=pos_sol;
			mejor_candidato=candidato[i];
			mejor_reco_x=min_reco_x;
			mejor_reco_y=min_reco_y;
			}
		else
			if(min_reco_x+min_reco_y<mejor_reco_x+mejor_reco_y)
				{
				mejor_pos_sol=pos_sol;
				mejor_candidato=candidato[i];
				mejor_reco_x=min_reco_x;
				mejor_reco_y=min_reco_y;
				}
			//x_act=asigna_posicion(it1,p1,pos_sol,pos,y_act);
			//y_min=solution_item[pos].y;
		}
	else
		{
		cont=calcula_posiciones_siguiente_item(it1,p1,x_min,x_act,y_min,y_max,x_max,pos);
		if(!cont)
			{
			//++p1; //Se buscara en una nueva hoja
			//x_max=x_min=y_max=y_min=0;
			cont=calcula_posiciones_primer_item(it1,p1+1,0,0,0);
			for(j=0;j<cont;++j)
				{
				arreglo_reco_x[j]=posible_lugar[j].x;
				arreglo_reco_y[j]=posible_lugar[j].y;
				}
			if(!cont)
				{
				//printf("Algo anda MUY MAL, NO SE PUDO ENCONTRAR UNA POSICION PARA UN ITEM\n");
				continue;
				//break;
				}
			}
		tipo=posible_lugar[0].tipo;
		x1=posible_lugar[0].x;
		y1=posible_lugar[0].y;
		if(tipo) //Si vale 1 está transpuesto
			{
			x1+=item[it1].length;
			y1+=item[it1].witdth;
			}
		else//Si vale 0 esta en su posicion original
			{
			x1+=item[it1].witdth;
			y1+=item[it1].length;
			}
		pos_sol=0;
		min_x_posicion=x1;
		//max_y_posicion=y1;
		min_tipo_posicion=posible_lugar[0].tipo_new_posicion;
		min_reco_x=arreglo_reco_x[0];
		min_reco_y=arreglo_reco_y[0];
		for(j=1;j<cont;++j)
			{
			tipo=posible_lugar[j].tipo;
			x1=posible_lugar[j].x;
			y1=posible_lugar[j].y;
			if(tipo) //Si vale 1 está transpuesto
				{
				x1+=item[it1].length;
				y1+=item[it1].witdth;
				}
			else//Si vale 0 esta en su posicion original
				{
				x1+=item[it1].witdth;
				y1+=item[it1].length;
				}
			tipo_posicion=posible_lugar[j].tipo_new_posicion;
			if((tipo_posicion==POSICION_DER || tipo_posicion==POSICION_ARRIBA) && x1>x_max)
				continue;
			if(tipo_posicion==POSICION_DER)
				{
				dif_y=y_max-y_min;
				dif_y=(double)dif_y*factor;
				cota_y=y_min+dif_y;
				if(y1>cota_y)
					continue;
				}
				//Checa las
				ban_compara=compara(tipo_posicion,x1,arreglo_reco_x[j]+arreglo_reco_y[j],min_tipo_posicion,min_x_posicion,min_reco_x+min_reco_y,x_max,y1,cota_y);
				if(ban_compara==1) //Si gano el nuevo en comparacion con el mejor anterior
						{
						min_reco_x=arreglo_reco_x[j];
						min_reco_y=arreglo_reco_y[j];
						min_x_posicion=x1;
						min_tipo_posicion=tipo_posicion;
						pos_sol=j;
						}
						//}
				}
			
			tipo=posible_lugar[pos_sol].tipo;
			x1=posible_lugar[pos_sol].x;
			y1=posible_lugar[pos_sol].y;
			if(tipo) //Si vale 1 está transpuesto
				{
				x1+=item[it1].length;
				y1+=item[it1].witdth;
				}
			else//Si vale 0 esta en su posicion original
				{
				x1+=item[it1].witdth;
				y1+=item[it1].length;
				}
			if(min_tipo_posicion==POSICION_DER)
				{
				dif_y=y_max-y_min;
				dif_y=(double)dif_y*factor;
				cota_y=y_min+dif_y;
				if(mejor_pos_sol!=-1 && y1>cota_y && ban_forzoso)
						continue;
				}
			if(mejor_pos_sol==-1)
				{
				mejor_pos_sol=pos_sol;
				mejor_candidato=candidato[i];
				mejor_tipo_posicion=min_tipo_posicion;
				mejor_x_posicion=min_x_posicion;
				mejor_reco_x=min_reco_x;
				mejor_reco_y=min_reco_y;
				}
			else
			/*	if(min_reco_x+min_reco_y<mejor_reco_x+mejor_reco_y)
					{
					mejor_pos_sol=pos_sol;
					mejor_candidato=candidato[i];
					mejor_tipo_posicion=min_tipo_posicion;
					mejor_x_posicion=min_x_posicion;
					mejor_reco_x=min_reco_x;
					mejor_reco_y=min_reco_y;
					}
				else
					if(min_reco_x+min_reco_y==mejor_reco_x+mejor_reco_y)*/
						{
						ban_compara=compara02(min_tipo_posicion,min_x_posicion,min_reco_x+min_reco_y,mejor_tipo_posicion,mejor_x_posicion,mejor_reco_x+mejor_reco_y,x_max,y1,cota_y);
						if(ban_compara==1) //Si gano el nuevo en comparacion con el mejor anterior
							{
							mejor_pos_sol=pos_sol;
							mejor_candidato=candidato[i];
							mejor_tipo_posicion=min_tipo_posicion;
							mejor_x_posicion=min_x_posicion;
							mejor_reco_x=min_reco_x;
							mejor_reco_y=min_reco_y;
							}
						}
			}
		} //Fin del for(i=0;i<cont_items && !ban;++i)
	
return mejor_pos_sol;
}

//Esta funcion compara entre 2 posibles posiciones del mismo o diferentes items y decide cual utilizar pide que no se rebase x_max en caso
//de que la posicion sea a la derecha o arriba del ultimo item
int compara01B(int tipo_pos01,int x1,int desplaza01,int tipo_pos02,int x2,int desplaza02,int x_max,int y1,int cota_y,int y2)
{
int ban=0;

if(tipo_pos01==ANT_POSICION_NULA)
	{
	if(tipo_pos02==ANT_POSICION_NULA)
		{
		if(desplaza01<desplaza02)
			ban=1; //Gana el 1
		else
			if(desplaza01>desplaza02)
				ban=-1;//Gana el 2
			else
				{
				if(y1>y2)
					ban=1; //Gana el 1
				else
					if(y2>y1)
						ban=-1; //Gana el 2
					else
						{
						if(x1>x2)
							ban=-1;
						if(x1<x2)
							ban=1; //Gana el 1
						}
				}
		}
	else//Cualquier posicion es mejor que agarrar una nueva hoja
		ban=-1;
	}
if(tipo_pos01==POSICION_NUEVA)
	{
	if(tipo_pos02==ANT_POSICION_NULA)
		ban=1; //Gana el 1
	else
		if(tipo_pos02==POSICION_NUEVA)
			{
			if(desplaza01<desplaza02)
				ban=1; //Gana el 1
			else
				if(desplaza01>desplaza02)
					ban=-1; //Gana el 2
				else
					{
					if(y1>y2)
						ban=1; //Gana el 1
					else
						if(y2>y1)
							ban=-1; //Gana el 2
						else
							{
							if(x1>x2)
								ban=-1;
							if(x1<x2)
								ban=1; //Gana el 1
							}
					}
			}
		else //Puede ser POSICION_DER O POSICION_ARRIBA
			{
			if(x2<=x_max)
				ban=-1; //Gana el x2
			else  
				//if(x1<=x_max)//Gana el x1 (1)
					ban=1;  //Aqui pierde por sobrepasar x_max, pero se puede modificar para que lo sobrepase en máximo un porcentaje o el que lo sobrepase menos
			/*	else
					{
					if(x1>x2)
						ban=-1;
					if(x1<x2)
						ban=1; //Gana el 1
					}*/
			}
	}
if(tipo_pos01==POSICION_DER)
	{
	if(tipo_pos02==ANT_POSICION_NULA)
		ban=1; //Gana el 1
	else
		if(tipo_pos02==POSICION_NUEVA)
			{
/*			if(cota_y!=-1 && y1>cota_y)
				ban=-1; //Gana el 2
			else
				{*/
				if(x1<=x_max)
					ban=1; //Gane el 1
				else
					ban=-1;
			//	}
			}
		else
			if(tipo_pos02==POSICION_DER)
				{
				if(desplaza01<desplaza02)
					ban=1; //Gana el 1
				else
					if(desplaza01>desplaza02)
						ban=-1;
					else
						{
						if(cota_y!=-1 && y1>cota_y)
							ban=-1; //Gana el 2
						else
							{
							if(y1>y2)
								ban=1; //Gana el 1
							else
								if(y2>y1)
									ban=-1; //Gana el 2
								else
									{
									if(x1>x2)
										ban=-1; //Gana el 2
									if(x1<x2)
										ban=1; //Gana el 1
									}
							}
						}
				}
			else
				{
				if(x1<=x_max)
					ban=1; //Gana el 1
				else
					ban=-1;
				}
	}
if(tipo_pos01==POSICION_ARRIBA)
	{
	if(tipo_pos02==ANT_POSICION_NULA)
		ban=1; //Gana el 1
	else
		if(tipo_pos02==POSICION_NUEVA)
			{
			if(x1<=x_max)
				ban=1; //Gane el 1
			else
				ban=-1;
			}
		else
			if(tipo_pos02==POSICION_DER)
				{
				if(x2<=x_max)
					ban=-1;
				else
					if(x1<=x_max)
						ban=1;
				}
			else
				if(tipo_pos02==POSICION_ARRIBA)
					{
					if(desplaza01<desplaza02)
						ban=1; //Gana el 1
					else
						if(desplaza01>desplaza02)
							ban=-1; //Gana el 2
						else
							{
							if(y1>y2)
								ban=1; //Gana el 1
							else
								if(y2>y1) 
									ban=-1; //Gana el 2
								else
									{
									if(x1>x2)
										ban=-1; //Gana el 2
									if(x1<x2)
										ban=1; //Gana el 1
									}
							}
					}
	}
return ban;
}

//Esta funcion compara entre 2 posibles posiciones del mismo o diferentes items y decide cual utilizar pide que no se rebase x_max en caso
//de que la posicion sea a la derecha o arriba del ultimo item
int compara01C(int tipo_pos01,int x1,int desplaza01,int tipo_pos02,int x2,int desplaza02,int x_max,int y1,int cota_y)
{
int ban=0;

if(tipo_pos01==ANT_POSICION_NULA)
	{
	if(tipo_pos02==ANT_POSICION_NULA)
		{
		if(desplaza01<desplaza02)
			ban=1; //Gana el 1
		else
			if(desplaza01>desplaza02)
				ban=-1;//Gana el 2
			else
				{
				if(x1>x2)
					ban=1;
				if(x1<x2)
					ban=-1; //Gana el 2
				}
		}
	else//Cualquier posicion es mejor que agarrar una nueva hoja
		ban=-1;
	}
if(tipo_pos01==POSICION_NUEVA)
	{
	if(tipo_pos02==ANT_POSICION_NULA)
		ban=1; //Gana el 1
	else
		if(tipo_pos02==POSICION_NUEVA)
			{
			if(desplaza01<desplaza02)
				ban=1; //Gana el 1
			else
				if(desplaza01>desplaza02)
					ban=-1; //Gana el 2
				else
					{
					if(x1>x2)
						ban=1; //Gana el 1
					if(x1<x2)
						ban=-1; //Gana el 2
					}
			}
		else //Puede ser POSICION_DER O POSICION_ARRIBA
			{
			if(x2<=x_max)
				ban=-1; //Gana el x2
			else  
				//if(x1<=x_max)//Gana el x1 (1)
					ban=1;  //Aqui pierde por sobrepasar x_max, pero se puede modificar para que lo sobrepase en máximo un porcentaje o el que lo sobrepase menos
			/*	else
					{
					if(x1>x2)
						ban=-1;
					if(x1<x2)
						ban=1; //Gana el 1
					}*/
			}
	}
if(tipo_pos01==POSICION_DER)
	{
	if(tipo_pos02==ANT_POSICION_NULA)
		ban=1; //Gana el 1
	else
		if(tipo_pos02==POSICION_NUEVA)
			{
/*			if(cota_y!=-1 && y1>cota_y)
				ban=-1; //Gana el 2
			else
				{*/
				if(x1<=x_max)
					ban=1; //Gane el 1
				else
					ban=-1;
			//	}
			}
		else
			if(tipo_pos02==POSICION_DER)
				{
				if(desplaza01<desplaza02)
					ban=1; //Gana el 1
				else
					if(desplaza01>desplaza02)
						ban=-1;
					else
						{
						if(cota_y!=-1 && y1>cota_y)
							ban=-1; //Gana el 2
						else
							{
							if(x1>x2)
								ban=1; //Gana el 2
							if(x1<x2)
								ban=-1; //Gana el 1
							}
						}
				}
			else
				{
				if(x1<=x_max)
					ban=1; //Gana el 1
				else
					ban=-1;
				}
	}
if(tipo_pos01==POSICION_ARRIBA)
	{
	if(tipo_pos02==ANT_POSICION_NULA)
		ban=1; //Gana el 1
	else
		if(tipo_pos02==POSICION_NUEVA)
			{
			if(x1<=x_max)
				ban=1; //Gane el 1
			else
				ban=-1;
			}
		else
			if(tipo_pos02==POSICION_DER)
				{
				if(x2<=x_max)
					ban=-1;
				else
					if(x1<=x_max)
						ban=1;
				}
			else
				if(tipo_pos02==POSICION_ARRIBA)
					{
					if(desplaza01<desplaza02)
						ban=1; //Gana el 1
					else
						if(desplaza01>desplaza02)
							ban=-1; //Gana el 2
						else
							{
							if(x1>x2)
								ban=1; //Gana el 2
							if(x1<x2)
								ban=-1; //Gana el 1
							}
					}
	}
return ban;
}



int encuentra_mejor_movimiento02(int cont_items,int &mejor_candidato,int pos,int p1,int x_min,int y_min,int x_max,int y_max,int x_act,double factor_y,int ban_forzoso,double factor_x,int combi01)
{
int mejor_pos_sol=-1,pos_sol,cota_y=-1,ban_compara,cota_x,dif_x;
int i,j,ban=0,it1,cont,tipo,tipo_posicion,mejor_tipo_posicion,mejor_x_posicion;
int x1,y1,min_x_posicion,min_reco_x,min_reco_y,mejor_reco_x,mejor_reco_y,min_tipo_posicion,dif_y,max_y_posicion,max_x_posicion;
dif_x=x_max-x_min;
cota_x=x_max+dif_x*factor_x;

for(i=0;i<cont_items && !ban;++i) //Elige el mejor candidato con su "mejor" posicion segun un criterio gloton
	{
	it1=candidato[i];
	if(pos==0) //Si es el primer elemento de una hoja
		{
		cont=calcula_posiciones_primer_item(it1,p1,0,0,0);
		for(j=0;j<cont;++j)
			{
			arreglo_reco_x[j]=posible_lugar[j].x;
			arreglo_reco_y[j]=posible_lugar[j].y;
			}
		if(!cont)
			continue;
		tipo=posible_lugar[0].tipo;
		x1=posible_lugar[0].x;
		y1=posible_lugar[0].y;
		if(tipo) //Si vale 1 está transpuesto
			{
			x1+=item[it1].length;
			y1+=item[it1].witdth;
			}
		else//Si vale 0 esta en su posicion original
			{
			x1+=item[it1].witdth;
			y1+=item[it1].length;
			}
		pos_sol=0;
		max_x_posicion=min_x_posicion=x1;
		max_y_posicion=y1;
		min_reco_x=arreglo_reco_x[0];
		min_reco_y=arreglo_reco_y[0];
		for(j=1;j<cont;++j) //Busca la posicion que incremente menos la x
			{
			tipo=posible_lugar[j].tipo;
			x1=posible_lugar[j].x;
			y1=posible_lugar[j].y;
			if(tipo) //Si vale 1 está transpuesto
				{
				x1+=item[it1].length;
				y1+=item[it1].witdth;
				}
			else//Si vale 0 esta en su posicion original
				{
				x1+=item[it1].witdth;
				y1+=item[it1].length;
				}
			if(arreglo_reco_x[j]+arreglo_reco_y[j]<min_reco_x+min_reco_y)
				{
				min_reco_x=arreglo_reco_x[j];
				min_reco_y=arreglo_reco_y[j];
				min_x_posicion=x1;
				max_y_posicion=y1;
				pos_sol=j;
				}
			else
				if(arreglo_reco_x[j]+arreglo_reco_y[j]==min_reco_x+min_reco_y)
					{
					if(combi01==0)
						{
						if(x1<min_x_posicion)
							{
							min_reco_x=arreglo_reco_x[j];
							min_reco_y=arreglo_reco_y[j];
							min_x_posicion=x1;
							max_y_posicion=y1;
							pos_sol=j;
							}
						}
					else
						if(y1>max_y_posicion)
							{
							min_reco_x=arreglo_reco_x[j];
							min_reco_y=arreglo_reco_y[j];
							min_x_posicion=x1;
							max_y_posicion=y1;
							pos_sol=j;
							}
					}
			}
			//ban=1; //Ya se eligió la mejor posicion :)
		if(mejor_pos_sol==-1)
			{
			mejor_pos_sol=pos_sol;
			mejor_candidato=candidato[i];
			mejor_reco_x=min_reco_x;
			mejor_reco_y=min_reco_y;
			}
		else
			if(min_reco_x+min_reco_y<mejor_reco_x+mejor_reco_y)
				{
				mejor_pos_sol=pos_sol;
				mejor_candidato=candidato[i];
				mejor_reco_x=min_reco_x;
				mejor_reco_y=min_reco_y;
				}
			//x_act=asigna_posicion(it1,p1,pos_sol,pos,y_act);
			//y_min=solution_item[pos].y;
		}
	else
		{
		cont=calcula_posiciones_siguiente_item(it1,p1,x_min,x_act,y_min,y_max,x_max,pos);
		if(!cont)
			{
			//++p1; //Se buscara en una nueva hoja
			//x_max=x_min=y_max=y_min=0;
			cont=calcula_posiciones_primer_item(it1,p1+1,0,0,0);
			for(j=0;j<cont;++j)
				{
				arreglo_reco_x[j]=posible_lugar[j].x;
				arreglo_reco_y[j]=posible_lugar[j].y;
				}
			if(!cont)
				{
				//printf("Algo anda MUY MAL, NO SE PUDO ENCONTRAR UNA POSICION PARA UN ITEM\n");
				continue;
				//break;
				}
			}
		tipo=posible_lugar[0].tipo;
		x1=posible_lugar[0].x;
		y1=posible_lugar[0].y;
		if(tipo) //Si vale 1 está transpuesto
			{
			x1+=item[it1].length;
			y1+=item[it1].witdth;
			}
		else//Si vale 0 esta en su posicion original
			{
			x1+=item[it1].witdth;
			y1+=item[it1].length;
			}
		pos_sol=0;
		max_x_posicion=min_x_posicion=x1;
		max_y_posicion=y1;
		min_tipo_posicion=posible_lugar[0].tipo_new_posicion;
		min_reco_x=arreglo_reco_x[0];
		min_reco_y=arreglo_reco_y[0];
		for(j=1;j<cont;++j)
			{
			tipo=posible_lugar[j].tipo;
			x1=posible_lugar[j].x;
			y1=posible_lugar[j].y;
			if(tipo) //Si vale 1 está transpuesto
				{
				x1+=item[it1].length;
				y1+=item[it1].witdth;
				}
			else//Si vale 0 esta en su posicion original
				{
				x1+=item[it1].witdth;
				y1+=item[it1].length;
				}
			tipo_posicion=posible_lugar[j].tipo_new_posicion;
			if((tipo_posicion==POSICION_DER || tipo_posicion==POSICION_ARRIBA) && x1>x_max)
				continue;
			if(tipo_posicion==POSICION_DER)
				{
				dif_y=y_max-y_min;
				dif_y=(double)dif_y*factor_y;
				cota_y=y_min+dif_y;
				if(y1>cota_y)
					continue;
				}
				//Checa las
			switch (combi01)
			{
			case 0:
				ban_compara=compara(tipo_posicion,x1,arreglo_reco_x[j]+arreglo_reco_y[j],min_tipo_posicion,min_x_posicion,min_reco_x+min_reco_y,x_max,y1,cota_y);
				break;
			case 1:
				ban_compara=compara01B(tipo_posicion,x1,arreglo_reco_x[j]+arreglo_reco_y[j],min_tipo_posicion,min_x_posicion,min_reco_x+min_reco_y,cota_x,y1,cota_y,max_y_posicion);
				break;
			case 2:
				ban_compara=compara01C(tipo_posicion,x1,arreglo_reco_x[j]+arreglo_reco_y[j],min_tipo_posicion,max_x_posicion,min_reco_x+min_reco_y,x_max,y1,cota_y);
			default:
				break;
			}
							
				if(ban_compara==1) //Si gano el nuevo en comparacion con el mejor anterior
						{
						min_reco_x=arreglo_reco_x[j];
						min_reco_y=arreglo_reco_y[j];
						min_x_posicion=x1;
						max_x_posicion=x1;
						min_tipo_posicion=tipo_posicion;
						pos_sol=j;
						}
						//}
				}
			
			tipo=posible_lugar[pos_sol].tipo;
			x1=posible_lugar[pos_sol].x;
			y1=posible_lugar[pos_sol].y;
			if(tipo) //Si vale 1 está transpuesto
				{
				x1+=item[it1].length;
				y1+=item[it1].witdth;
				}
			else//Si vale 0 esta en su posicion original
				{
				x1+=item[it1].witdth;
				y1+=item[it1].length;
				}
			if(min_tipo_posicion==POSICION_DER)
				{
				dif_y=y_max-y_min;
				dif_y=(double)dif_y*factor_y;
				cota_y=y_min+dif_y;
				if(mejor_pos_sol!=-1 && y1>cota_y && ban_forzoso)
						continue;
				}
			if(mejor_pos_sol==-1)
				{
				mejor_pos_sol=pos_sol;
				mejor_candidato=candidato[i];
				mejor_tipo_posicion=min_tipo_posicion;
				mejor_x_posicion=min_x_posicion;
				mejor_reco_x=min_reco_x;
				mejor_reco_y=min_reco_y;
				}
			else
			/*	if(min_reco_x+min_reco_y<mejor_reco_x+mejor_reco_y)
					{
					mejor_pos_sol=pos_sol;
					mejor_candidato=candidato[i];
					mejor_tipo_posicion=min_tipo_posicion;
					mejor_x_posicion=min_x_posicion;
					mejor_reco_x=min_reco_x;
					mejor_reco_y=min_reco_y;
					}
				else
					if(min_reco_x+min_reco_y==mejor_reco_x+mejor_reco_y)*/
						{
						ban_compara=compara02(min_tipo_posicion,min_x_posicion,min_reco_x+min_reco_y,mejor_tipo_posicion,mejor_x_posicion,mejor_reco_x+mejor_reco_y,x_max,y1,cota_y);
						if(ban_compara==1) //Si gano el nuevo en comparacion con el mejor anterior
							{
							mejor_pos_sol=pos_sol;
							mejor_candidato=candidato[i];
							mejor_tipo_posicion=min_tipo_posicion;
							mejor_x_posicion=min_x_posicion;
							mejor_reco_x=min_reco_x;
							mejor_reco_y=min_reco_y;
							}
						}
			}
		} //Fin del for(i=0;i<cont_items && !ban;++i)
	
return mejor_pos_sol;
}

void guarda_mejor_solucion()
{
int i,it1;
for(i=0;i<num_items;++i)
	{
	mejor_solucion[i]=solution_item[i];
	it1=solution_item[i].id;
	sol_best[i].it1=it1;
	sol_best[i].pos_sol=backtraking[i].pos_sol;
	}
}

void utiliza_mejor_solucion()
{
int i;
for(i=0;i<num_items;++i)
	{
	solution_item[i]=mejor_solucion[i];
	sol_actual[i]=sol_best[i];
	}
}

void gloton10(char archivo[])
{
double factor_x,factor_y,r;
long int costo,ban_forzoso;
int p1=0,x_min=0,y_min=0,x_max=0,y_max=0,x_act=0,y_act=0,combi01;
int cont_items,pos=0,cont,it1,pos_sol,mejor_candidato,mejor_pos_sol,tipo_ord;
inicializa_variables();
	tipo_ord=rand()%3;
	combi01=rand()%3;
while(pos<num_items)
	{

	switch(tipo_ord)
		{
		case 0:
			cont_items=calcula_candidatos_lado_corto_mas_largo();
			break;
		case 1:
			cont_items=calcula_candidatos_lado_mas_largo();
			break;
		case 2:
			cont_items=calcula_candidatos_mayor_area();
			break;
		}
	r =rand()%102;
	if(r==101)
		factor_y=20;
	else
		factor_y=1.0+r/100;
	/*r=rand()%102;
	if(r==101)
		factor_x=20;
	else
		factor_x=r/1000;*/
	factor_x=0.1;
	ban_forzoso=rand()%2;
	mejor_pos_sol=-1;
	mejor_pos_sol=encuentra_mejor_movimiento02(cont_items,mejor_candidato,pos,p1,x_min,y_min,x_max,y_max,x_act,factor_y,ban_forzoso,factor_x,combi01);

	it1=mejor_candidato;
	if(pos==0)
		cont=calcula_posiciones_primer_item(it1,p1,x_min,y_min,0);
	else
		{
		cont=calcula_posiciones_siguiente_item(it1,p1,x_min,x_act,y_min,y_max,x_max,pos);
		if(!cont)
			{
			++p1;
			x_max=x_min=y_max=y_min=0;
			cont=calcula_posiciones_primer_item(it1,p1,0,0,0);
			if(!cont)
					{
					//printf("Algo anda MUY REQUETE MAL, NO SE PUDO ENCONTRAR UNA POSICION PARA UN ITEM\n");
					continue;
					//break;
					}
			}
		}
	backtraking[pos].pos_sol=pos_sol=mejor_pos_sol;
	x_act=asigna_posicion(it1,p1,pos_sol,pos,y_act);
	y_min=solution_item[pos].y;
	if(posible_lugar[pos_sol].tipo_new_posicion==POSICION_NUEVA)
		{
		//y_min=solution_item[i].y;
		x_min=solution_item[pos].x;
		y_max=y_min;
		if(solution_item[pos].tipo) //Si esta tranpuesto
			y_max+=item[it1].witdth;
		else //Si no esta transpuesto
			y_max+=item[it1].length;
		}
	if(x_act>x_max)
		x_max=x_act;
	if(y_act>y_max)
		y_max=y_act;
	bandera[it1]=1;
	++pos;
	}
costo=calcula_costo();
//printf("Costo obtenido: %ld\n",costo);
if(mejor_costo>costo)
	{
	mejor_costo=costo;
	min_p1=p1;
	max_x_cota=x_max;
	guarda_mejor_solucion();
	//++num_sol;
	}
}

//Esta funcion es para 2 items diferentes
//Esta funcion compara entre 2 posibles posiciones del mismo o diferentes items y decide cual utilizar pide que no se rebase x_max en caso
//de que la posicion sea a la derecha o arriba del ultimo item
int compara02B(int tipo_pos01,int x1,int desplaza01,int tipo_pos02,int x2,int desplaza02,int x_max,int y1,int cota_y,int y2)
{
int ban=0;
if(tipo_pos01==ANT_POSICION_NULA)
	{
	if(tipo_pos02==ANT_POSICION_NULA)
		{
		if(desplaza01<desplaza02)
			ban=1; //Gane el 1
		else
			if(desplaza01>desplaza02)
				ban=-1; //Gana el 2
			else
				{
				if(y1>y2)
					ban=1; //Gana el 1
				else
					if(y2>y1)
						ban=-1; //Gana el 2
					else
						{
						if(x1<x2)
							ban=-1;
						if(x1>x2)
							ban=1; //Gana el 1
						}
				}
		}
	else//Cualquier posicion es mejor que agarrar una nueva hoja
		ban=-1;
	}
if(tipo_pos01==POSICION_NUEVA)
	{
	if(tipo_pos02==ANT_POSICION_NULA)
		ban=1; //Gana el 1
	else
		if(tipo_pos02==POSICION_NUEVA)
			{
			if(desplaza01<desplaza02)
				ban=1; //Gana el 1
			else
				if(desplaza01>desplaza02)
					ban=-1;
				else
					{
					if(y1>y2)
						ban=1; //Gana el 1
					else
						if(y2>y1)
							ban=-1; //Gana el 2
						else
							{
							if(x1<x2)
								ban=-1;
							if(x1>x2)
								ban=1; //Gana el 1
							}
					}
			}
		else //Puede ser POSICION_DER O POSICION_ARRIBA
			{
			if(x2<=x_max)
				ban=-1; //Gana el x2
			else  
				//if(x1<=x_max)//Gana el x1 (1)
					ban=1;  //Aqui pierde por sobrepasar x_max, pero se puede modificar para que lo sobrepase en máximo un porcentaje o el que lo sobrepase menos
			/*	else
					{
					if(x1>x2)
						ban=-1;
					if(x1<x2)
						ban=1; //Gana el 1
					}*/
			}
	}
if(tipo_pos01==POSICION_DER)
	{
	if(tipo_pos02==ANT_POSICION_NULA)
		ban=1; //Gana el 1
	else
		if(tipo_pos02==POSICION_NUEVA)
			{
			if(x1<=x_max)
				ban=1; //Gane el 1
			else
				ban=-1;
			}
		else
			if(tipo_pos02==POSICION_DER)
				{
				if(desplaza01<desplaza02)
					ban=1; //Gana el 1
				else
					if(desplaza01>desplaza02)
						ban=-1;
					else
						{
						if(cota_y!=-1 && y1>cota_y)
							ban=-1; //Gana el 2
						else
							{
							if(x1<=x_max && x2>x_max)
								ban=1;//Gana el 1
							else
								if(x1>x_max && x2<=x_max)
									ban=-1;//Gana el 2
								else
									{
									if(y1>y2)
										ban=1; //Gana el 1
									else
										if(y2>y1)
											ban=-1; //Gana el 2
										else
											{
											if(x1>x2)
												ban=1; //Gana el 1
											if(x1<x2)
												ban=-1; //Gana el 2
											}
									}
							}
						}
				}
			else
				{
				if(x1<=x_max)
					ban=1; //Gana el 1
				else
					ban=-1;
				}
	}
if(tipo_pos01==POSICION_ARRIBA)
	{
	if(tipo_pos02==ANT_POSICION_NULA)
		ban=1; //Gana el 1
	else
		if(tipo_pos02==POSICION_NUEVA)
			{
			if(x1<=x_max)
				ban=1; //Gane el 1
			else
				ban=-1;
			}
		else
			if(tipo_pos02==POSICION_DER)
				{
				if(x2<=x_max)
					ban=-1;
				else
					if(x1<=x_max)
						ban=1;
				}
			else
				if(tipo_pos02==POSICION_ARRIBA)
					{
					if(desplaza01<desplaza02)
						ban=1; //Gana el 1
					else
						if(desplaza01>desplaza02)
							ban=-1; //Gana el 2
						else
							{
							if(x1<=x_max && x2>x_max)
								ban=1;//Gana el 1
							else
								if(x1>x_max && x2<=x_max)
									ban=-1;//Gana el 2
								else
									{
									if(y1>y2)
										ban=1; //Gana el 1
									else
										if(y2>y1)
											ban=-1; //Gana el 2
										else
											{
											if(x1>x2)
												ban=1; //Gana el 1
											if(x1<x2)
												ban=-1; //Gana el 2
											}
									}
							}
					}
	}
return ban;
}

int encuentra_mejor_movimiento03(int cont_items,int &mejor_candidato,int pos,int p1,int x_min,int y_min,int x_max,int y_max,int x_act,double factor_y,int ban_forzoso,double factor_x,int combi01,int combi02,int &mejor_tipo_posicion)
{
int mejor_pos_sol=-1,pos_sol,cota_y=-1,ban_compara,cota_x,dif_x;
int i,j,ban=0,it1,cont,tipo,tipo_posicion,mejor_x_posicion,mejor_y_posicion;
int x1,y1,min_x_posicion,min_reco_x,min_reco_y,mejor_reco_x,mejor_reco_y,min_tipo_posicion,dif_y,max_y_posicion,max_x_posicion;
dif_x=x_max-x_min;
cota_x=x_max+dif_x*factor_x;

for(i=0;i<cont_items && !ban;++i) //Elige el mejor candidato con su "mejor" posicion segun un criterio gloton
	{
	it1=candidato[i];
	if(pos==0) //Si es el primer elemento de una hoja
		{
		cont=calcula_posiciones_primer_item(it1,p1,0,0,0);
		for(j=0;j<cont;++j)
			{
			arreglo_reco_x[j]=posible_lugar[j].x;
			arreglo_reco_y[j]=posible_lugar[j].y;
			}
		if(!cont)
			continue;
		tipo=posible_lugar[0].tipo;
		x1=posible_lugar[0].x;
		y1=posible_lugar[0].y;
		if(tipo) //Si vale 1 está transpuesto
			{
			x1+=item[it1].length;
			y1+=item[it1].witdth;
			}
		else//Si vale 0 esta en su posicion original
			{
			x1+=item[it1].witdth;
			y1+=item[it1].length;
			}
		pos_sol=0;
		max_x_posicion=min_x_posicion=x1;
		max_y_posicion=y1;
		min_reco_x=arreglo_reco_x[0];
		min_reco_y=arreglo_reco_y[0];
		for(j=1;j<cont;++j) //Busca la posicion que incremente menos la x
			{
			tipo=posible_lugar[j].tipo;
			x1=posible_lugar[j].x;
			y1=posible_lugar[j].y;
			if(tipo) //Si vale 1 está transpuesto
				{
				x1+=item[it1].length;
				y1+=item[it1].witdth;
				}
			else//Si vale 0 esta en su posicion original
				{
				x1+=item[it1].witdth;
				y1+=item[it1].length;
				}
			if(arreglo_reco_x[j]+arreglo_reco_y[j]<min_reco_x+min_reco_y)
				{
				min_reco_x=arreglo_reco_x[j];
				min_reco_y=arreglo_reco_y[j];
				min_x_posicion=x1;
				max_y_posicion=y1;
				pos_sol=j;
				}
			else
				if(arreglo_reco_x[j]+arreglo_reco_y[j]==min_reco_x+min_reco_y)
					{
					if(combi01==0)
						{
						if(x1<min_x_posicion)
							{
							min_reco_x=arreglo_reco_x[j];
							min_reco_y=arreglo_reco_y[j];
							min_x_posicion=x1;
							max_y_posicion=y1;
							pos_sol=j;
							}
						}
					else
						if(y1>max_y_posicion)
							{
							min_reco_x=arreglo_reco_x[j];
							min_reco_y=arreglo_reco_y[j];
							min_x_posicion=x1;
							max_y_posicion=y1;
							pos_sol=j;
							}
					}
			}
			//ban=1; //Ya se eligió la mejor posicion :)
		if(mejor_pos_sol==-1)
			{
			mejor_pos_sol=pos_sol;
			mejor_candidato=candidato[i];
			mejor_reco_x=min_reco_x;
			mejor_reco_y=min_reco_y;
			}
		else
			if(min_reco_x+min_reco_y<mejor_reco_x+mejor_reco_y)
				{
				mejor_pos_sol=pos_sol;
				mejor_candidato=candidato[i];
				mejor_reco_x=min_reco_x;
				mejor_reco_y=min_reco_y;
				}
			//x_act=asigna_posicion(it1,p1,pos_sol,pos,y_act);
			//y_min=solution_item[pos].y;
		mejor_tipo_posicion=ANT_POSICION_NULA;
		}
	else
		{
		cont=calcula_posiciones_siguiente_item(it1,p1,x_min,x_act,y_min,y_max,x_max,pos);
		if(!cont)
			{
			//++p1; //Se buscara en una nueva hoja
			//x_max=x_min=y_max=y_min=0;
			cont=calcula_posiciones_primer_item(it1,p1+1,0,0,0);
			for(j=0;j<cont;++j)
				{
				arreglo_reco_x[j]=posible_lugar[j].x;
				arreglo_reco_y[j]=posible_lugar[j].y;
				}
			if(!cont)
				{
				//printf("Algo anda MUY MAL, NO SE PUDO ENCONTRAR UNA POSICION PARA UN ITEM\n");
				continue;
				//break;
				}
			}
		tipo=posible_lugar[0].tipo;
		x1=posible_lugar[0].x;
		y1=posible_lugar[0].y;
		if(tipo) //Si vale 1 está transpuesto
			{
			x1+=item[it1].length;
			y1+=item[it1].witdth;
			}
		else//Si vale 0 esta en su posicion original
			{
			x1+=item[it1].witdth;
			y1+=item[it1].length;
			}
		pos_sol=0;
		max_x_posicion=min_x_posicion=x1;
		max_y_posicion=y1;
		min_tipo_posicion=posible_lugar[0].tipo_new_posicion;
		min_reco_x=arreglo_reco_x[0];
		min_reco_y=arreglo_reco_y[0];
		for(j=1;j<cont;++j)
			{
			tipo=posible_lugar[j].tipo;
			x1=posible_lugar[j].x;
			y1=posible_lugar[j].y;
			if(tipo) //Si vale 1 está transpuesto
				{
				x1+=item[it1].length;
				y1+=item[it1].witdth;
				}
			else//Si vale 0 esta en su posicion original
				{
				x1+=item[it1].witdth;
				y1+=item[it1].length;
				}
			tipo_posicion=posible_lugar[j].tipo_new_posicion;
			if((tipo_posicion==POSICION_DER || tipo_posicion==POSICION_ARRIBA) && x1>x_max)
				continue;
			if(tipo_posicion==POSICION_DER)
				{
				dif_y=y_max-y_min;
				dif_y=(double)dif_y*factor_y;
				cota_y=y_min+dif_y;
				if(y1>cota_y)
					continue;
				}
				//Checa las
			switch (combi01)
			{
			case 0:
				ban_compara=compara(tipo_posicion,x1,arreglo_reco_x[j]+arreglo_reco_y[j],min_tipo_posicion,min_x_posicion,min_reco_x+min_reco_y,x_max,y1,cota_y);
				break;
			case 1:
				ban_compara=compara01B(tipo_posicion,x1,arreglo_reco_x[j]+arreglo_reco_y[j],min_tipo_posicion,min_x_posicion,min_reco_x+min_reco_y,cota_x,y1,cota_y,max_y_posicion);
				break;
			case 2:
				ban_compara=compara01C(tipo_posicion,x1,arreglo_reco_x[j]+arreglo_reco_y[j],min_tipo_posicion,max_x_posicion,min_reco_x+min_reco_y,x_max,y1,cota_y);
			default:
				break;
			}
							
				if(ban_compara==1) //Si gano el nuevo en comparacion con el mejor anterior
						{
						min_reco_x=arreglo_reco_x[j];
						min_reco_y=arreglo_reco_y[j];
						min_x_posicion=x1;
						max_x_posicion=x1;
						min_tipo_posicion=tipo_posicion;
						pos_sol=j;
						}
						//}
				}
			
			tipo=posible_lugar[pos_sol].tipo;
			x1=posible_lugar[pos_sol].x;
			y1=posible_lugar[pos_sol].y;
			if(tipo) //Si vale 1 está transpuesto
				{
				x1+=item[it1].length;
				y1+=item[it1].witdth;
				}
			else//Si vale 0 esta en su posicion original
				{
				x1+=item[it1].witdth;
				y1+=item[it1].length;
				}
			if(min_tipo_posicion==POSICION_DER)
				{
				dif_y=y_max-y_min;
				dif_y=(double)dif_y*factor_y;
				cota_y=y_min+dif_y;
				if(mejor_pos_sol!=-1 && y1>cota_y && ban_forzoso)
						continue;
				}
			if(mejor_pos_sol==-1)
				{
				mejor_pos_sol=pos_sol;
				mejor_candidato=candidato[i];
				mejor_tipo_posicion=min_tipo_posicion;
				mejor_x_posicion=min_x_posicion;
				mejor_y_posicion=y1;
				mejor_reco_x=min_reco_x;
				mejor_reco_y=min_reco_y;
				}
			else
			/*	if(min_reco_x+min_reco_y<mejor_reco_x+mejor_reco_y)
					{
					mejor_pos_sol=pos_sol;
					mejor_candidato=candidato[i];
					mejor_tipo_posicion=min_tipo_posicion;
					mejor_x_posicion=min_x_posicion;
					mejor_reco_x=min_reco_x;
					mejor_reco_y=min_reco_y;
					}
				else
					if(min_reco_x+min_reco_y==mejor_reco_x+mejor_reco_y)*/
						{
						if(combi02==0)
							ban_compara=compara02(min_tipo_posicion,min_x_posicion,min_reco_x+min_reco_y,mejor_tipo_posicion,mejor_x_posicion,mejor_reco_x+mejor_reco_y,x_max,y1,cota_y);
						else
							ban_compara=compara02B(min_tipo_posicion,min_x_posicion,min_reco_x+min_reco_y,mejor_tipo_posicion,mejor_x_posicion,mejor_reco_x+mejor_reco_y,x_max,y1,cota_y,mejor_y_posicion);
						if(ban_compara==1) //Si gano el nuevo en comparacion con el mejor anterior
							{
							mejor_pos_sol=pos_sol;
							mejor_candidato=candidato[i];
							mejor_tipo_posicion=min_tipo_posicion;
							mejor_x_posicion=min_x_posicion;
							mejor_y_posicion=y1;
							mejor_reco_x=min_reco_x;
							mejor_reco_y=min_reco_y;
							}
						}
			}
		} //Fin del for(i=0;i<cont_items && !ban;++i)
return mejor_pos_sol;
}


void gloton12(char archivo[])
{
double factor_x,factor_y,r;
long int costo,ban_forzoso;
int p1=0,x_min=0,y_min=0,x_max=0,y_max=0,x_act=0,y_act=0,combi01,combi02,mejor_tipo_posicion;
int cont_items,pos=0,cont,it1,pos_sol,mejor_candidato,mejor_pos_sol,tipo_ord;
inicializa_variables();
	tipo_ord=rand()%3;
	combi01=rand()%3;
	combi02=rand()%2;
r =rand()%102;
		if(r==101)
			factor_y=20;
		else
			factor_y=1.0+r/100;
		r=rand()%12;
		if(r==102)
			factor_x=20;
		else
			factor_x=r/1000;		
while(pos<num_items)
	{
	switch(tipo_ord)
		{
		case 0:
			cont_items=calcula_candidatos_lado_corto_mas_largo();
			break;
		case 1:
			cont_items=calcula_candidatos_lado_mas_largo();
			break;
		case 2:
			cont_items=calcula_candidatos_mayor_area();
			break;
		}
	ban_forzoso=rand()%2;
	mejor_pos_sol=-1;
	mejor_pos_sol=encuentra_mejor_movimiento03(cont_items,mejor_candidato,pos,p1,x_min,y_min,x_max,y_max,x_act,factor_y,ban_forzoso,factor_x,combi01,combi02,mejor_tipo_posicion);
	if(pos && (mejor_tipo_posicion==ANT_POSICION_NULA || mejor_tipo_posicion==POSICION_NUEVA))
		{
		r =rand()%102;
		if(r==101)
			factor_y=20;
		else
			factor_y=1.0+r/100;
		r=rand()%12;
		if(r==102)
			factor_x=20;
		else
			factor_x=r/1000;
		combi01=rand()%3;
		combi02=rand()%2;
		mejor_pos_sol=encuentra_mejor_movimiento03(cont_items,mejor_candidato,pos,p1,x_min,y_min,x_max,y_max,x_act,factor_y,ban_forzoso,factor_x,combi01,combi02,mejor_tipo_posicion);
		}
	it1=mejor_candidato;
	if(pos==0)
		cont=calcula_posiciones_primer_item(it1,p1,x_min,y_min,0);
	else
		{
		cont=calcula_posiciones_siguiente_item(it1,p1,x_min,x_act,y_min,y_max,x_max,pos);
		if(!cont)
			{
			++p1;
			x_max=x_min=y_max=y_min=0;
			cont=calcula_posiciones_primer_item(it1,p1,0,0,0);
			if(!cont)
					{
					//printf("Algo anda MUY REQUETE MAL, NO SE PUDO ENCONTRAR UNA POSICION PARA UN ITEM\n");
					continue;
					//break;
					}
			}
		}
	backtraking[pos].pos_sol=pos_sol=mejor_pos_sol;
	x_act=asigna_posicion(it1,p1,pos_sol,pos,y_act);
	y_min=solution_item[pos].y;
	if(posible_lugar[pos_sol].tipo_new_posicion==POSICION_NUEVA)
		{
		//y_min=solution_item[i].y;
		x_min=solution_item[pos].x;
		y_max=y_min;
		if(solution_item[pos].tipo) //Si esta tranpuesto
			y_max+=item[it1].witdth;
		else //Si no esta transpuesto
			y_max+=item[it1].length;
		}
	if(x_act>x_max)
		x_max=x_act;
	if(y_act>y_max)
		y_max=y_act;
	bandera[it1]=1;
	++pos;
	}
costo=calcula_costo();
//printf("Costo obtenido: %ld\n",costo);
if(mejor_costo>costo)
	{
	mejor_costo=costo;
	min_p1=p1;
	max_x_cota=x_max;
	/*num_lineas=construye_solucion();
	imprime_solucion(archivo,num_sol,num_lineas,costo);
	
	final_global = time(NULL);
	segundos = difftime(final_global,inicio_global);
	imprime_debugger(archivo,costo,segundos+suma_acu,num_sol);
	printf("Mejor costo %ld\n",mejor_costo);
	++num_sol;*/
	guarda_mejor_solucion();
	}
}

long int genera_solucion_inicial()
{
long int costo;
int i,ban=1,cont,it1,pos,ban_primero=1,p1=0,x_min=0,x_max=0,y_min=0,y_max=0,x_act,y_act;
//Inicializa la bandera
for(i=0;i<num_items;++i)
	bandera[i]=0;
//Cuenta las pilas (conjunto de ordenes)
for(i=0;i<MAX_ITEMS && ban;++i)
	if(L_STACK[i].num)
		num_stacks=i+1;
	//else
		//break;
//Cuenta los plates (el # de hojas de vidrio grandes que se cortaran)
ban=1;
for(i=0;i<num_defectos && ban;++i)
	if(L_DEFECT[i].num)
		++num_plates;
	else
		ban=0;
//Genera la solucion inicial,
for(i=0;i<num_items;++i)
	{
	cont=calcula_candidatos();
	pos=rand()%cont;
	if(ban_primero)//Esto es para asegurarse que sera programado primero el primer item
		pos=0;
	it1=candidato[pos];
	sol_actual[i].it1=it1;
	if(ban_primero) //Si es el primer elemento de una hoja
		{
		ban_primero=0;
		cont=calcula_posiciones_primer_item(it1,p1,x_min,y_min,0);
		pos=rand()%cont;
		sol_actual[i].pos_sol=pos;
		x_act=asigna_posicion(it1,p1,pos,i,y_act);
		y_min=solution_item[i].y;
		}
	else
		{
		cont=calcula_posiciones_siguiente_item(it1,p1,x_min,x_act,y_min,y_max,x_max,i);
		if(!cont)
			{
			++p1; //Se buscara en una nueva hoja
			x_max=x_min=y_max=y_min=0;
			cont=calcula_posiciones_primer_item(it1,p1,x_min,y_min,0);
			//if(!cont)
			//	printf("Algo anda MUY MAL, NO SE PUDO ENCONTRAR UNA POSICION PARA UN ITEM\n");
			}
		pos=rand()%cont;
		sol_actual[i].pos_sol=pos;
		x_act=asigna_posicion(it1,p1,pos,i,y_act);
		y_min=solution_item[i].y;
		//if(posible_lugar[pos].tipo_new_posicion==POSICION_ARRIBA)//No se si debo comentariar esto o no
			//y_min=y_max;
		if(posible_lugar[pos].tipo_new_posicion==POSICION_NUEVA)
			{
			y_min=solution_item[i].y;
			x_min=solution_item[i].x;
			y_max=y_min;
			if(solution_item[i].tipo) //Si esta tranpuesto
				y_max+=item[it1].witdth;
			else //Si no esta transpuesto
				y_max+=item[it1].length;
			}
		
		}
	if(x_act>x_max)
		x_max=x_act;
	if(y_act>y_max)
		y_max=y_act;
	bandera[it1]=1;
	}
costo=calcula_costo();
return costo;
}


int siguiente_corte_uno(int ini,int fin,int x_ini,int &x_fin,int &type,int &new_ini,int &dif_min)
{
int i,ban=0,x_max=0,it1,x_act,cont=0,it_posible=-1,x_item_min=WIDTH_PLATE+1;
int width,length;
dif_min=0;
for(i=ini;i<fin;++i)
	{
	x_act=solution_item[i].x;
	it1=solution_item[i].id;
	if(solution_item[i].tipo==0)
		x_act+=item[it1].witdth;
	else
		x_act+=item[it1].length;
	if(solution_item[i].tipo_new_posicion==POSICION_NUEVA && i!=ini)
		{
		ban=1;
		break;
		}	
	else
		if(x_act>=x_ini)
			{
			ban=1;
			it_posible=it1;
			++cont;
			if(x_act>x_max)
				x_max=x_act;
			if(solution_item[i].x<x_item_min)
				x_item_min=solution_item[i].x;
			}
	}
new_ini=i; //Se avanza con la posicion inicial
if(ban)
	x_fin=x_max;
else
	x_max=x_fin=WIDTH_PLATE;
if(cont>1)
	{
	if(x_item_min>x_ini)
		{//Hay que hacer varios ajustes
		type=-1;
		x_fin=x_item_min;
		width=x_fin-x_ini;
		new_ini=ini;
		}
	else
		{
		type=-2;
		dif_min=encuentra_cota_corte_uno(new_ini,x_max);
		if(dif_min)
				x_fin=x_max+dif_min; //solution_item[new_ini].x;
		width=x_fin-x_ini;
		if(WIDTH_PLATE-x_fin<MIN_WASTE)
			x_max=x_fin=WIDTH_PLATE;
		}
	}
else
	if(cont==1)
		{
		it1=it_posible;
		if(new_ini!=fin)
			x_fin=solution_item[new_ini].x;
		width=x_fin-x_ini;
		length=HEIGH_PLATE;
		if(x_item_min>x_ini)
			{//Hay que hacer varios ajustes
			type=-1;
			x_fin=x_item_min;
			width=x_fin-x_ini;
			new_ini=ini;
			}
		else
			if((item[it1].witdth==width && item[it1].length==length)  || (item[it1].witdth==length && item[it1].length==width))
				type=it1;
			else
				type=-2;

		}
	else
		if(x_max-x_ini>=MIN_CORTE_TIPO_UNO && x_max==WIDTH_PLATE)
			type=-3;
		else
			type=-1;
if(x_max==WIDTH_PLATE)
	ban=0;
else
	ban=1;
return ban;
}

long int calcula_ajuste_costo_corte_uno(int p1)
{
long int costo=0;
int x_ini=0,x_fin=0,type,ban,new_ini,ini=0,fin,dif_min,dif_ant=0;
calcula_inicio_fin_items_plate(ini,fin,p1);
do {
	ban=siguiente_corte_uno(ini,fin,x_ini,x_fin,type,new_ini,dif_min);
	if(type==-3) //Este es un ajuste para lo del desperdicio
		type=-1;
	else
		dif_ant=dif_min;
	ini=new_ini;
	x_ini=x_fin;
	}
while(ban);
if(dif_ant)
	costo=dif_ant*HEIGH_PLATE;
return costo;
}

long int ajuste_costo()
{
long int costo;
int num_used_plates;
num_used_plates=calcula_plates_usados();
costo=calcula_ajuste_costo_corte_uno(num_used_plates-1);
return costo;
}

//Esta funcion calcula una solucion, dada una posicion
int calcula_solucion(int pos_it1)
{
int i,ban=1,p1=0,x_min=0,x_max=0,y_min=0,y_max=0,x_act,y_act,pos,it1,cont;
//Inicializa la bandera
for(i=0;i<num_items;++i)
	bandera[i]=0;
for(i=0;i<num_items && ban;++i)
	{
	it1=sol_actual[i].it1;
	if(i==0) //Si es el primer elemento de una hoja
		{
		cont=calcula_posiciones_primer_item(it1,p1,x_min,y_min,0);
		pos=sol_actual[i].pos_sol;
		if(pos>=cont)
			if(pos_it1==i)
				ban=0;
			else
				pos=cont-1;
		x_act=asigna_posicion(it1,p1,pos,i,y_act);
		y_min=solution_item[i].y;
		}
	else
		{
		cont=calcula_posiciones_siguiente_item(it1,p1,x_min,x_act,y_min,y_max,x_max,i);
		if(!cont)
			{
			++p1; //Se buscara en una nueva hoja
			x_max=x_min=y_max=y_min=0;
			cont=calcula_posiciones_primer_item(it1,p1,x_min,y_min,0);
			if(!cont)
				{
				ban=0;
				/*++p1; //Se buscara en una nueva hoja
				x_max=x_min=y_max=y_min=0;
				cont=calcula_posiciones_primer_item(it1,p1,x_min,y_min,0);
				if(!cont)
					printf("Algo anda MUY MAL, NO SE PUDO ENCONTRAR UNA POSICION PARA UN ITEM\n");*/
				}
			}
		pos=sol_actual[i].pos_sol;
		if(pos>=cont)
			if(pos_it1==i)
				ban=0;
			else
				pos=cont-1;
		x_act=asigna_posicion(it1,p1,pos,i,y_act);
		y_min=solution_item[i].y;
		if(posible_lugar[pos].tipo_new_posicion==POSICION_NUEVA)
			{
			y_min=solution_item[i].y;
			x_min=solution_item[i].x;
			y_max=y_min;
			if(solution_item[i].tipo) //Si esta tranpuesto
				y_max+=item[it1].witdth;
			else //Si no esta transpuesto
				y_max+=item[it1].length;
			}
		
		}
	if(x_act>x_max)
		x_max=x_act;
	if(y_act>y_max)
		y_max=y_act;
	bandera[it1]=1;
	}
return ban;
}


//Esta funcion calcula una solucion, dada una posicion
int calcula_solucion(int num_items_combinados,int &pos_item_combinado)
{
int i,ban=1,p1=0,x_min=0,x_max=0,y_min=0,y_max=0,x_act,y_act,pos,it1,cont;
pos_item_combinado=0;
//Inicializa la bandera
for(i=0;i<num_items;++i)
	bandera[i]=0;
for(i=0;i<num_items && ban;++i)
	{
	it1=sol_actual[i].it1;
	if(i==0) //Si es el primer elemento de una hoja
		{
		cont=calcula_posiciones_primer_item(it1,p1,x_min,y_min,0);
		pos=sol_actual[i].pos_sol;
		if(pos>=cont)
			if(item_combinado[pos_item_combinado]==i)
				{
				ban=0;
				break;
				}
			else
				pos=cont-1;
		x_act=asigna_posicion(it1,p1,pos,i,y_act);
		y_min=solution_item[i].y;
		}
	else
		{
		cont=calcula_posiciones_siguiente_item(it1,p1,x_min,x_act,y_min,y_max,x_max,i);
		if(!cont)
			{
			++p1; //Se buscara en una nueva hoja
			x_max=x_min=y_max=y_min=0;
			cont=calcula_posiciones_primer_item(it1,p1,x_min,y_min,0);
			if(!cont)
				ban=0;
			}
		pos=sol_actual[i].pos_sol;
		if(pos>=cont)
			if(item_combinado[pos_item_combinado]==i)
				{
				ban=0;
				break;
				}
			else
				pos=cont-1;
		x_act=asigna_posicion(it1,p1,pos,i,y_act);
		y_min=solution_item[i].y;
		if(posible_lugar[pos].tipo_new_posicion==POSICION_NUEVA)
			{
			y_min=solution_item[i].y;
			x_min=solution_item[i].x;
			y_max=y_min;
			if(solution_item[i].tipo) //Si esta tranpuesto
				y_max+=item[it1].witdth;
			else //Si no esta transpuesto
				y_max+=item[it1].length;
			}
		
		}
	if(x_act>x_max)
		x_max=x_act;
	if(y_act>y_max)
		y_max=y_act;
	bandera[it1]=1;
	if(item_combinado[pos_item_combinado]==i)
		++pos_item_combinado;
	}
return ban;
}

int calcula_ini_fin_pos_it1(int &ini,int &fin,int pos_it1)
{
vertice *p,*q;
int it1,st1,ban=0,cont=0,it_ant,it_after;
it1=sol_actual[pos_it1].it1;
st1=item[it1].stack;
if(L_STACK[st1].num>1)
	{
	p=&L_STACK[st1];
	q=p->next;
	while(p!=NULL && q->id!=it1) 
		{
		++cont;
		p=p->next;
		q=q->next;
		}
	if(cont)
		{
		it_ant=p->id;
		//BUsca la posicion del it_ant
		for(ini=pos_it1;sol_actual[ini].it1!=it_ant;--ini);
		++ini;
		}
	else
		ini=0;
	if(q->next!=NULL)
		{
		p=q->next;
		it_after=p->id;
		//Busca la posicion del it_after
		for(fin=pos_it1;sol_actual[fin].it1!=it_after;++fin);
		}
	else
		fin=num_items;
	if(fin-ini>1)
		ban=1;
	else
		ban=0;
	}
else
	{
	ini=0;
	fin=num_items;
	ban=1;
	}
return ban;
}

void guarda(struct_sol sol02[],struct_sol sol01[])
{
int i;
for(i=0;i<num_items;++i)
	sol02[i]=sol01[i];
}

void recorre(int pos1,int pos2)
{
struct_sol aux;
int i;
if(pos1<pos2)
	{
	aux=sol_actual[pos2];
	for(i=pos2;i>pos1;--i)
		sol_actual[i]=sol_actual[i-1];
	sol_actual[pos1]=aux;
	}
else
	{
	aux=sol_actual[pos2];
	for(i=pos2;i<pos1;++i)
		sol_actual[i]=sol_actual[i+1];
	sol_actual[pos1]=aux;
	}
}

//Esta funcion, dada una estructura de solucion y una posicion, indica si la nueva posicion del item es factible
//Si la posicion es factible, regresa 1, en caso contrario regresa 0
int factible_pos(struct_sol sol01[],int pos_it1)
{
vertice *p,*q;
int ban=1,it1,st1,cont=0,it_ant,ban_encontro=0,ini,fin,it_after;
it1=sol_actual[pos_it1].it1;
st1=item[it1].stack;
if(L_STACK[st1].num>1)
	{
	p=&L_STACK[st1];
	q=p->next;
	while(p!=NULL && q->id!=it1) 
		{
		++cont;
		p=p->next;
		q=q->next;
		}
	if(cont)
		{
		it_ant=p->id;
		//BUsca la posicion del it_ant
		for(ini=pos_it1;ini>=0 && !ban_encontro;--ini)
			if(sol_actual[ini].it1==it_ant)
				ban_encontro=1;
		if(!ban_encontro)
			ban=0;
		}
	else
		ban=1;
	if(q->next!=NULL && ban)
		{
		p=q->next;
		it_after=p->id;
		//Busca la posicion del it_after
		ban_encontro=0;
		for(fin=pos_it1;fin<num_items && !ban_encontro;++fin)
			if(sol_actual[fin].it1==it_after)
				ban_encontro=1;
		if(!ban_encontro)
			ban=0;
		}
	}
else
	{
	ini=0;
	fin=num_items;
	ban=1;
	}
return ban;
}

void inicializa_combinacion(int num_items_combinados)
{
int i;
for(i=0;i<num_items_combinados;++i)
		combinacion[i]=0;
}

int siguiente_combinacion(int num_items_combinados)
{
++combinacion[num_items_combinados-1];
return 1;
}

int siguiente_combinacion(int num_items_combinados,int pos_fallida)
{
int i,ban;
if(pos_fallida==0)
	ban=0;
else
	{
	--pos_fallida;
	++combinacion[pos_fallida];
	for(i=pos_fallida+1;i<num_items_combinados;++i)
		combinacion[i]=0;
	ban=1;
	}
return ban;
}

//Esta funcion efectua una busqueda local
int busca(long int &costo_actual)
{
double segundos;
struct_sol aux;
long int costo;
int i,j,k,pos_sol_ant,ban=0,ban_solution,it1,ini,fin,ban_exito,ban_sig,pos;
final_global = time(NULL);
segundos = difftime(final_global,inicio_global);				
//Primera vecindad, prueba las diferentes posiciones en que se pueden poner los items, sin modificar la secuencia en que es producido
for(i=0;i<num_items && !ban && segundos<max_time-1;++i)
	{
	pos_sol_ant=sol_actual[i].pos_sol;
	for(j=0;!ban && segundos<max_time-1;++j)
		{
		sol_actual[i].pos_sol=j;
		ban_solution=calcula_solucion(i);
		if(ban_solution)
			{
			costo=calcula_costo();
			if(costo<costo_actual)
				{
				costo_actual=costo;
				ban=1;
				}
			}
		else
			break;
		final_global = time(NULL);
		segundos = difftime(final_global,inicio_global);			
		}
	if(!ban)
		sol_actual[i].pos_sol=pos_sol_ant;
	}
//Segunda vecindad, cambia la secuencia (orden en que es poroducido un item) y prueba sus posiciones
for(i=num_items-1;i>=0 && !ban && segundos<max_time-1;--i)
	{
	//break;
	it1=sol_actual[i].it1;
    ban_exito=calcula_ini_fin_pos_it1(ini,fin,i);
	if(ban_exito)//Si hay mas de una posicion en que se puede mover
		{
		guarda(sol_respaldo,sol_actual);
		for(j=ini;j<fin && !ban && segundos<max_time-1;++j)
			if(i!=j)
				{//Cambia el orden
				guarda(sol_actual,sol_respaldo);
				recorre(j,i);
				//Ahora cambia el movimiento
				pos_sol_ant=sol_actual[j].pos_sol;
				for(k=0;!ban;++k)
					{
					sol_actual[j].pos_sol=k;
					ban_solution=calcula_solucion(j);
					if(ban_solution)
						{
						costo=calcula_costo();
						if(costo<costo_actual)
							{
							costo_actual=costo;
							ban=1;
							}
						}
					else
						break;
					}
				final_global = time(NULL);
				segundos = difftime(final_global,inicio_global);			
				}
		if(!ban)
			guarda(sol_actual,sol_respaldo);
		}//Fin del if(ban_exito)
	}
//Tercera vecindad, hace un swap!! 
guarda(sol_respaldo,sol_actual);
for(i=0;i<num_items-1 && !ban && segundos<max_time-1;++i)
	for(j=i+1;j<num_items && !ban && segundos<max_time-1;++j)
		{
//		if(i==57 && j==61)
//			ban=0;
		aux=sol_actual[i];
		sol_actual[i]=sol_actual[j];
		sol_actual[j]=aux;
		if(factible_pos(sol_actual,i) && factible_pos(sol_actual,j))
			{
			item_combinado[0]=i;
			item_combinado[1]=j;
			inicializa_combinacion(2);
			do {
				sol_actual[i].pos_sol=combinacion[0];
				sol_actual[j].pos_sol=combinacion[1];
				ban_solution=calcula_solucion(2,pos);
				if(ban_solution)
					{
					costo=calcula_costo();
					if(costo<costo_actual)
						{
						costo_actual=costo;
						ban=1;
						}
					ban_sig=siguiente_combinacion(2);
					}
				else
					ban_sig=siguiente_combinacion(2,pos);
				final_global = time(NULL);
				segundos = difftime(final_global,inicio_global);			
				}
			while(!ban && ban_sig && segundos<max_time-1);
			if(!ban)
				guarda(sol_actual,sol_respaldo);
			}
		else
			{
			aux=sol_actual[i];
			sol_actual[i]=sol_actual[j];
			sol_actual[j]=aux;
			final_global = time(NULL);
			segundos = difftime(final_global,inicio_global);				
			}
		}
return ban;
}

int busca02(long int &costo_actual)
{
double segundos;
long int costo;
int i,j,ban=0,ban_sig,ban_solution,pos;
//Primera vecindad, prueba las diferentes posiciones en que se pueden poner los items, sin modificar la secuencia en que es producido
final_global = time(NULL);
segundos = difftime(final_global,inicio_global);			
for(i=0;i<num_items && !ban && segundos<max_time-1;++i)
	for(j=i+1;j<num_items && !ban && segundos<max_time-1;++j)
		{
		guarda(sol_respaldo,sol_actual);
		item_combinado[0]=i;
		item_combinado[1]=j;
		inicializa_combinacion(2);
		do {
			sol_actual[i].pos_sol=combinacion[0];
			sol_actual[j].pos_sol=combinacion[1];
			ban_solution=calcula_solucion(2,pos);
			if(ban_solution)
				{
				costo=calcula_costo();
				if(costo<costo_actual)
					{
					costo_actual=costo;
					ban=1;
					}
				ban_sig=siguiente_combinacion(2);
				}
			else
				ban_sig=siguiente_combinacion(2,pos);
			final_global = time(NULL);
			segundos = difftime(final_global,inicio_global);			
			}
		while(!ban && ban_sig && segundos<max_time-1);
		if(!ban)
			guarda(sol_actual,sol_respaldo);
		}
return ban;
}

int calcula_porcentaje(double p[])
{
long int a1,area_usada=-1;
int i,p1=-1,plates_used=0,x_max=0,it1,x_act,dif,residuo=0;
a1=WIDTH_PLATE*HEIGH_PLATE;
for(i=0;i<num_items;++i)
	{
	x_act=solution_item[i].x;
	it1=solution_item[i].id;
	if(solution_item[i].tipo==0) //Esta en su posicion original
		x_act+=item[it1].witdth;
	else
		x_act+=item[it1].length;
	if(p1!=solution_item[i].plate)
		{
		if(area_usada>0)
			{
			p[plates_used-1]=(double)area_usada/(double)a1;
			}
		++plates_used;
		p1=solution_item[i].plate;
		area_usada=item[it1].length*item[it1].witdth;
		x_max=x_act;
		}
	else
		area_usada+=item[it1].length*item[it1].witdth;
	if(x_act>x_max)
		x_max=x_act;
	}
if(area_usada>0)
	{
	dif=WIDTH_PLATE-x_max;
	a1=HEIGH_PLATE*x_max;
	p[plates_used-1]=(double)area_usada/(double)a1;
	//++plates_used;
	}
return plates_used;
}

int diferentes(double a,double b)
{
if(a<b-EPSILON || a>b+EPSILON)
    return 1;
else
	return 0;
}

//Esta funcion recibe el porcentaje de dos soluciones regresa 1 si el primero es mayor en la primera diferencia de porcentajes
//Regresa 0 en caso contrario
int mayor(double p1[],double p2[],int n)
{
int ban=0,ban_dif=0,i;
for(i=0;i<n && !ban_dif;++i)
	if(diferentes(p1[i],p2[i]))
		{
		if(p1[i]>p2[i])
			ban=1;
		else
			ban=0;
		ban_dif=1;
		}
return ban;
}

int busca_movimiento_reacomodo(long int &costo_actual)
{
double segundos;
struct_sol aux;
long int costo;
int ban_encuentra=0,p1_ori,ban=0,it1,i,j,k,ban_exito,ini,fin,pos_sol_ant,ban_solution,num_plates;
int pos,ban_sig;
//Calcula el procentaje actual
num_plates=calcula_porcentaje(porcentaje);
final_global = time(NULL);
segundos = difftime(final_global,inicio_global);			
//Segunda vecindad, cambia la secuencia (orden en que es poroducido un item) y prueba sus posiciones
for(i=num_items-1;i>=0 && !ban && !ban_encuentra && segundos<max_time-1;--i)
	{
	//break;
	it1=sol_actual[i].it1;
	p1_ori=solution_item[i].plate;
    ban_exito=calcula_ini_fin_pos_it1(ini,fin,i);
	if(ban_exito)//Si hay mas de una posicion en que se puede mover
		{
		guarda(sol_respaldo,sol_actual);
		for(j=ini;j<fin && !ban && !ban_encuentra && segundos<max_time-1;++j)
			if(i!=j)
				{//Cambia el orden
				guarda(sol_actual,sol_respaldo);
				recorre(j,i);
				//Ahora cambia el movimiento
				pos_sol_ant=sol_actual[j].pos_sol;
				for(k=0;!ban && !ban_encuentra && segundos<max_time-1;++k)
					{
					sol_actual[j].pos_sol=k;
					ban_solution=calcula_solucion(j);
					if(ban_solution)
						{
						costo=calcula_costo();
						if(costo<costo_actual)
							{
							ban_encuentra=2;
							costo_actual=costo;
							ban=1;
							}
						else
							if(costo==costo_actual)
								{
								calcula_porcentaje(porcentaje02);
								ban_encuentra=mayor(porcentaje02,porcentaje,num_plates);
								}
						}
					else
						break;
					final_global = time(NULL);
					segundos = difftime(final_global,inicio_global);			
					}
				}
		if(!ban && !ban_encuentra)
			guarda(sol_actual,sol_respaldo);
		}//Fin del if(ban_exito)
	}
//Tercera vecindad, hace un swap!! 
guarda(sol_respaldo,sol_actual);
for(i=0;i<num_items-1 && !ban && !ban_encuentra && segundos<max_time-1;++i)
	for(j=i+1;j<num_items && !ban && !ban_encuentra && segundos<max_time-1;++j)
		{
		aux=sol_actual[i];
		sol_actual[i]=sol_actual[j];
		sol_actual[j]=aux;
		if(factible_pos(sol_actual,i) && factible_pos(sol_actual,j))
			{
			item_combinado[0]=i;
			item_combinado[1]=j;
			inicializa_combinacion(2);
			do {
				sol_actual[i].pos_sol=combinacion[0];
				sol_actual[j].pos_sol=combinacion[1];
				ban_solution=calcula_solucion(2,pos);
				if(ban_solution)
					{
					costo=calcula_costo();
					if(costo<costo_actual)
						{
						ban_encuentra=2;
						costo_actual=costo;
						ban=1;
						}
					else
						if(costo==costo_actual)
							{
							calcula_porcentaje(porcentaje02);
							ban_encuentra=mayor(porcentaje02,porcentaje,num_plates);
							//ban=1;
							}
					ban_sig=siguiente_combinacion(2);
					}
				else
					ban_sig=siguiente_combinacion(2,pos);
				final_global = time(NULL);
				segundos = difftime(final_global,inicio_global);			
				}
			while(!ban && ban_sig && !ban_encuentra && segundos<max_time-1);
			if(!ban && !ban_encuentra)
				guarda(sol_actual,sol_respaldo);
			}
		else
			{
			aux=sol_actual[i];
			sol_actual[i]=sol_actual[j];
			sol_actual[j]=aux;
			final_global = time(NULL);
			segundos = difftime(final_global,inicio_global);				
			}
		}
if(!ban && !ban_encuentra)
	calcula_solucion(0);
return ban_encuentra;
}

//Esta funcion inicializa la combinacion de items a combinar, tomandolos del arreglo bandera_tabu (se toman los marcados)
void inicializa_combinacion02(int num_items_combinados)
{
int i,cont=0,it1;
for(i=0;i<num_items_combinados;++i)
		combinacion[i]=0;
for(i=0;i<num_items && cont<num_items_combinados;++i)
	{
	it1=sol_actual[i].it1;
	if(bandera_tabu[it1])
		{
		item_combinado[cont]=i;
		++cont;
		}
	}
}

int busca02prima(long int &costo_actual,int nivel)
{
double segundos;
long int costo;
int i,j,k,ban=0,it1,ban_exito,ini,fin,ban_solution,pos,ban_sig;
//Segunda vecindad, cambia la secuencia (orden en que es poroducido un item) y prueba sus posiciones
final_global = time(NULL);
segundos = difftime(final_global,inicio_global);									
for(i=num_items-1;i>=0 && !ban && segundos<max_time-1;--i)
	{
	//break;
	it1=sol_actual[i].it1;
    ban_exito=calcula_ini_fin_pos_it1(ini,fin,i);
	if(ban_exito && !bandera_tabu[it1])//Si hay mas de una posicion en que se puede mover
		{
		//if(nivel==0)//Hay que comentariar esto
			//k=0;//Hay que comentariar esto
		bandera_tabu[it1]=1;
		guarda(res_sol_nivel[nivel],sol_actual);
		for(j=ini;j<fin && !ban && segundos<max_time-1;++j)
			//if(i!=j)
				{//Cambia el orden
				guarda(sol_actual,res_sol_nivel[nivel]);
				recorre(j,i);
				if(nivel+1<max_nivel)
					{
					ban_solution=busca02prima(costo_actual,nivel+1);
					final_global = time(NULL);
					segundos = difftime(final_global,inicio_global);			
					if(ban_solution)
						ban=1;
					}
				else
					{
					inicializa_combinacion02(max_nivel);
					guarda(res_sol_nivel[nivel+1],sol_actual);
					do {
						for(k=0;k<max_nivel;++k)
							{
							pos=item_combinado[k];
							sol_actual[pos].pos_sol=combinacion[k];
							}
						ban_solution=calcula_solucion(max_nivel,pos);
						if(ban_solution)
							{
							costo=calcula_costo();
							if(costo<costo_actual)
								{
								costo_actual=costo;
								ban=1;
								}
							ban_sig=siguiente_combinacion(max_nivel);
							}
						else
							ban_sig=siguiente_combinacion(max_nivel,pos);
						final_global = time(NULL);
						segundos = difftime(final_global,inicio_global);			
						}
					while(!ban && ban_sig && segundos<max_time-1);
					}
				} //Fin del if(i!=j)
		if(!ban)
			guarda(sol_actual,res_sol_nivel[nivel]);
		bandera_tabu[it1]=0;
		}//Fin del if(ban_exito)
	}
return ban;
}

int busca02prima02(long int &costo_actual,int nivel)
{
double segundos;
struct_sol aux;
long int costo;
int i,j,ban=0,ban_solution,pos,it1,it2,n,k,ban_sig;
n=(nivel+1)*2;
//Tercera vecindad, hace un swap!! 
final_global = time(NULL);
segundos = difftime(final_global,inicio_global);			
guarda(res_sol_nivel[nivel],sol_actual);
for(i=0;i<num_items && !ban && segundos<max_time-1;++i)
	{
	it1=sol_actual[i].it1;
	if(!bandera_tabu[it1])
		for(j=i+1;j<num_items && !ban && segundos<max_time-1;++j)
			{
			it2=sol_actual[j].it1;
			if(!bandera_tabu[it2])
				{
				aux=sol_actual[i];
				sol_actual[i]=sol_actual[j];
				sol_actual[j]=aux;
				if(factible_pos(sol_actual,i) && factible_pos(sol_actual,j))
					{
					bandera_tabu[it1]=1;
					bandera_tabu[it2]=1;
					if(nivel+1<max_nivel)
						{
						ban_solution=busca02prima02(costo_actual,nivel+1);
						if(ban_solution)
							ban=1;
						}
					else
						{
						inicializa_combinacion02(n);
						guarda(res_sol_nivel[nivel+1],sol_actual);
						do {
							for(k=0;k<n;++k)
								{
								pos=item_combinado[k];
								sol_actual[pos].pos_sol=combinacion[k];
								}
							ban_solution=calcula_solucion(n,pos);
							if(ban_solution)
								{
								costo=calcula_costo();
								if(costo<costo_actual)
									{
									costo_actual=costo;
									ban=1;
									}
								ban_sig=siguiente_combinacion(n);
								}
							else
								ban_sig=siguiente_combinacion(n,pos);
							final_global = time(NULL);
							segundos = difftime(final_global,inicio_global);			
							}
						while(!ban && ban_sig && segundos<max_time-1);
						if(!ban)
							guarda(sol_actual,res_sol_nivel[nivel]);
						} //fin del else del if(nivel+1<max_nivel)
					bandera_tabu[it1]=0;
					bandera_tabu[it2]=0;
					} //Fin del if(factible_pos(sol_actual,i) && factible_pos(sol_actual,j))
				else
					{
					aux=sol_actual[i];
					sol_actual[i]=sol_actual[j];
					sol_actual[j]=aux;
					} //Fin del else del if(factible_pos(sol_actual,i) && factible_pos(sol_actual,j))
				}//Fin del if(!bandera_tabu[it2])
			}//Fin del for(j=i+1;j<num_items && !ban;++j)
	}//Fin del for(i=0;i<num_items && !ban;++i)
return ban;
}

int busqueda_local(char archivo[])
{
double segundos;
long int costo,costo_ant;
int i,cont=0,ban=0,ban_reacomodo,num_lineas;
for(i=0;i<num_items;++i)
	bandera_tabu[i]=0;
if(mejor_costo==max_costo)
	{
	costo=genera_solucion_inicial();
	mejor_costo=costo;
	//printf("Hay un costo INICIAL de %ld\n",costo);
	guarda_mejor_solucion();
	}
else
	costo=calcula_costo();
final_global = time(NULL);
segundos = difftime(final_global,inicio_global);
do {
	final_global = time(NULL);
	segundos = difftime(final_global,inicio_global);
	if(segundos<max_time-1)
		ban=busca(costo);
	else
		break;
	final_global = time(NULL);
	segundos = difftime(final_global,inicio_global);
	if(ban)
		{
		costo_ant=costo;
		costo+=ajuste_costo();
		if(mejor_costo>costo)
			{
			mejor_costo=costo;
			guarda_mejor_solucion();
			/*if(costo!=costo_ant)
				printf("%ld (%ld) ",costo_ant,costo);
			else
				printf("%ld ",costo);*/
			costo=costo_ant;
			}
		else
			{
			//mejor_costo=costo_ant;
			//printf("%ld (%ld) ",costo_ant,costo);
			costo=costo_ant;
			}
		cont=0;
		}
	else
		if(segundos<max_time-1)
			{
			//printf("\nTrying Reacomodo: ");
			ban_reacomodo=0;
			calcula_solucion(0);
			do {
				ban=busca_movimiento_reacomodo(costo);
				if(ban==1 || ban==2)
					ban_reacomodo=1;
				final_global = time(NULL);
				segundos = difftime(final_global,inicio_global);
				if(ban==2)
					break;
				}
			while(ban && segundos<max_time-1);
			if(ban_reacomodo)
				{
				costo_ant=costo=calcula_costo();
				costo+=ajuste_costo();
				if(costo<mejor_costo)
					{
					mejor_costo=costo;
					guarda_mejor_solucion();
					//imprime(archivo,num_sol);
					//++num_sol;
					//printf("REACO: %ld\n",costo);
					}
				else
					{
					//mejor_costo=costo_ant;
					//printf("%ld (%ld) ",costo_ant,costo);
					costo=costo_ant;
					}
				cont=0;
				}
			else
				{
				final_global = time(NULL);
				segundos = difftime(final_global,inicio_global);
				if(segundos<max_time-1)
					{
					ban=busca02(costo);
					if(ban)
						{
						mejor_costo=costo;
						guarda_mejor_solucion();
						//printf("%ld ",mejor_costo);
						cont=0;
						}
					else
						{
						final_global = time(NULL);
						segundos = difftime(final_global,inicio_global);
						if(segundos<max_time-1)
							ban=busca02prima(costo,0);
						else
							break;
						if(ban)
							{
							//printf("EXITO ");
							//costo=calcula_costo();
							mejor_costo=costo;
							guarda_mejor_solucion();
						//	printf("%ld ",mejor_costo);
							cont=0;
							}
						else
							{
							ban=busca02prima02(costo,0);
							if(ban)
								{
								//printf("EXITO_SWAP02 ");
								mejor_costo=costo;
								guarda_mejor_solucion();
								//printf("%ld ",mejor_costo);
								cont=0;
								}
							else
								++cont;
							}
						}
					}
				else
					break;
				if(!ban)
					++cont;
				}
		}
	}
while(cont<1 && segundos<max_time-1);
utiliza_mejor_solucion();
	num_lineas=construye_solucion();
	min_p1=impresion_solucion[num_lineas-1].plate;
	max_x_cota=impresion_solucion[num_lineas-1].x;
return min_p1;
}

int hacia_adelante(int pos)
{
double segundos;
long int costo_parcial;
int cont,it1,p1,x_min=0,y_min=0,x_max,y_max,pos_it1,pos_sol,x_act,y_act;
x_min=backtraking[pos].x_min;
y_min=backtraking[pos].y_min;
x_max=backtraking[pos].x_max;
y_max=backtraking[pos].y_max;
x_act=backtraking[pos].x_act;
p1=backtraking[pos].plate;
final_global = time(NULL);
segundos = difftime(final_global,inicio_global);				
while(pos<num_items && segundos<max_time-1)
	{
	cont=calcula_candidatos();
	pos_it1=0;
	it1=candidato[pos_it1];
	pos_sol=0;
	if(pos==0) //Si es el primer elemento de una hoja
		{
		cont=calcula_posiciones_primer_item(it1,p1,x_min,y_min,0);
		x_act=asigna_posicion(it1,p1,pos_sol,pos,y_act);
		y_min=solution_item[pos].y;
		}
	else
		{
		cont=calcula_posiciones_siguiente_item(it1,p1,x_min,x_act,y_min,y_max,x_max,pos);
		if(!cont)
			{
			++p1; //Se buscara en una nueva hoja
			x_max=x_min=y_max=y_min=0;
			cont=calcula_posiciones_primer_item(it1,p1,x_min,y_min,0);
			if(!cont)
				break;
			}
		x_act=asigna_posicion(it1,p1,pos_sol,pos,y_act);
		y_min=solution_item[pos].y;
		if(posible_lugar[pos_sol].tipo_new_posicion==POSICION_NUEVA)
			{
			//y_min=solution_item[i].y;
			x_min=solution_item[pos].x;
			y_max=y_min;
			if(solution_item[pos].tipo) //Si esta tranpuesto
				y_max+=item[it1].witdth;
			else //Si no esta transpuesto
				y_max+=item[it1].length;
			}
		}
	//Primero guarda los valores antes de aumentar pos
	backtraking[pos].pos_it1=pos_it1;
	backtraking[pos].pos_sol=pos_sol;
	if(x_act>x_max)
		x_max=x_act;
	if(y_act>y_max)
		y_max=y_act;
	bandera[it1]=1;
	++pos;
	backtraking[pos].x_act=x_act;
	backtraking[pos].x_min=x_min;
	backtraking[pos].y_min=y_min;
	backtraking[pos].x_max=x_max;
	backtraking[pos].y_max=y_max;
	backtraking[pos].plate=p1;
	if(p1>min_p1 || (p1==min_p1 && x_max>max_x_cota))
		break;
	if(mejor_costo!=-1 && pos && (solution_item[pos-1].tipo_new_posicion==POSICION_NUEVA || (solution_item[pos-1].tipo_new_posicion==ANT_POSICION_NULA && pos-1>0)))
		{
		costo_parcial=calcula_costo_parcial(pos-1);
		if(costo_parcial>mejor_costo)
			break;
		}
	final_global = time(NULL);
	segundos = difftime(final_global,inicio_global);			
	}
return pos;
}

int hacia_atras(int &pos)
{
double segundos;
long int costo_parcial;
int cont,it1,p1,x_min=0,y_min=0,x_max,y_max,pos_it1,pos_sol,ban=0,x_act,y_act,cont_candidatos,ban_p1=0;
--pos;
final_global = time(NULL);
segundos = difftime(final_global,inicio_global);				
while(!ban && pos>=0 && segundos<max_time-1)
	{
	it1=solution_item[pos].id;
	bandera[it1]=0;
	cont_candidatos=calcula_candidatos();
	pos_it1=backtraking[pos].pos_it1;
	pos_sol=0;
	do {
		x_min=backtraking[pos].x_min;
		y_min=backtraking[pos].y_min;
		x_max=backtraking[pos].x_max;
		y_max=backtraking[pos].y_max;
		x_act=backtraking[pos].x_act;
		p1=backtraking[pos].plate;
		it1=candidato[pos_it1];
		if(pos_it1>=cont_candidatos)
			break;
		pos_sol=backtraking[pos].pos_sol+1;
		backtraking[pos].pos_sol=-1;
		if(pos==0) //Si es el primer elemento de una hoja
			{
			cont=calcula_posiciones_primer_item(it1,p1,x_min,y_min,0);
			if(pos_sol>=cont)
				{
				++pos_it1;
				continue;
				}
			x_act=asigna_posicion(it1,p1,pos_sol,pos,y_act);
			y_min=solution_item[pos].y;
			}
		else
			{
			cont=calcula_posiciones_siguiente_item(it1,p1,x_min,x_act,y_min,y_max,x_max,pos);
			if(!cont)
				{
				++p1; //Se buscara en una nueva hoja
				x_max=x_min=y_max=y_min=0;
				cont=calcula_posiciones_primer_item(it1,p1,x_min,y_min,0);
				}
			if(pos_sol>=cont)
				{
				++pos_it1;
				continue;
				}
			x_act=asigna_posicion(it1,p1,pos_sol,pos,y_act);
			y_min=solution_item[pos].y;
			if(posible_lugar[pos_sol].tipo_new_posicion==POSICION_NUEVA)
				{
			//y_min=solution_item[i].y;
				x_min=solution_item[pos].x;
				y_max=y_min;
				if(solution_item[pos].tipo) //Si esta tranpuesto
					y_max+=item[it1].witdth;
				else //Si no esta transpuesto
					y_max+=item[it1].length;
				}
			}
		if(x_act>x_max)
			x_max=x_act;
		if(y_act>y_max)
			y_max=y_act;
		bandera[it1]=1;
		ban=1;
		}
	while(!ban);
	if(ban)
		{
		backtraking[pos].pos_it1=pos_it1;
		backtraking[pos].pos_sol=pos_sol;
		++pos;
		backtraking[pos].x_act=x_act;
		backtraking[pos].x_min=x_min;
		backtraking[pos].y_min=y_min;
		backtraking[pos].x_max=x_max;
		backtraking[pos].y_max=y_max;
		backtraking[pos].plate=p1;
		if(p1<min_p1 || (p1==min_p1 && x_max<max_x_cota))
			{
			ban=1;
			if(mejor_costo!=-1 && solution_item[pos-1].tipo_new_posicion==POSICION_NUEVA)
				{
				costo_parcial=calcula_costo_parcial(pos-1);
				if(costo_parcial>mejor_costo)
					{
					ban=0;
					--pos;
					}
				}
			}
		else
			{
			ban=0;
			--pos;
			}
		}
	else
		--pos;
	final_global = time(NULL);
	segundos = difftime(final_global,inicio_global);			
	}
return ban;
}

void exacto(char archivo[])
{
double segundos;
long int costo,cont_itera=0;
int pos=0,ban=1,ban_primero=1;
inicializa_variables();
final_global = time(NULL);
segundos = difftime(final_global,inicio_global);			
while(ban && segundos<max_time-1)
	{
	final_global = time(NULL);
	segundos = difftime(final_global,inicio_global);			
	if(segundos<max_time-1)
		pos=hacia_adelante(pos);
	else
		break;
	if(pos==num_items)
		{
		costo=calcula_costo();
		if(costo<mejor_costo || mejor_costo==-1)
			{
			mejor_costo=costo;
			guarda_mejor_solucion();
			if(backtraking[num_items].plate<min_p1 || (backtraking[num_items].plate==min_p1 && backtraking[num_items].x_max<max_x_cota))
				{
				min_p1=backtraking[num_items].plate;
				max_x_cota=backtraking[num_items].x_max;
				}
			}
		}

	do {
		ban=hacia_atras(pos);
		++cont_itera;
		if(pos==num_items)
			{
			costo=calcula_costo();
			if(costo<mejor_costo || mejor_costo==-1)
				{
				mejor_costo=costo;
				guarda_mejor_solucion();
				if(backtraking[num_items].plate<min_p1 || (backtraking[num_items].plate==min_p1 && backtraking[num_items].x_max<max_x_cota))
					{
					min_p1=backtraking[num_items].plate;
					max_x_cota=backtraking[num_items].x_max;
					}
				}
			}//FIN del if(pos==num_items)
		final_global = time(NULL);
		segundos = difftime(final_global,inicio_global);			
		}
	while(ban && pos==num_items && segundos<max_time-1);
	}
}

void procesa(char archivo[])
{
double segundos=0;
int i;
//printf("Gloton 10:\n");
for(i=0;i<40000 && segundos<max_time-10;++i)
	{
	gloton10(archivo);
	final_global = time(NULL);
	segundos = difftime(final_global,inicio_global);
	}
//printf("Gloton 12:\n");
for(i=0;i<100000 && segundos<max_time-10;++i)
	{
	gloton12(archivo);
	final_global = time(NULL);
	segundos = difftime(final_global,inicio_global);
	}
utiliza_mejor_solucion();
//printf("Empieza la busqueda local\n");
busqueda_local(archivo);
utiliza_mejor_solucion();
//printf("\nEmpieza el algoritmo exacto:\n");
exacto(archivo);
}

//Esta funcion imprime el costo y el tiempo en un archivo .out
void imprime(char archivo[])
{
double segundos;
long int costo;
int num_lineas;
costo=calcula_costo();
num_lineas=construye_solucion();
imprime_solucion(archivo,num_lineas,costo);
final_global = time(NULL);
segundos = difftime(final_global,inicio_global);
}

int main(int argc,char *argv[])
{
char archivo[160]="V2.toy005.xml",salida[160]="V2.toy005_SOL.xml";  //fact//
double segundos;
long int semilla;
int ban,i,ban_archivo=0,ban_salida=0,ban_name=0,ban_time=0;
inicio_global = time(NULL);  //Toma el tiempo al inicio
//Primero procesa los argumentos
if(argc==1 && ((*argv)[1])!='-')
	{
	printf("\n Wrong Syntax: ./executable -t time_limit -p instance_filename -o new_solution_filename -name -s seed ");
	exit(1);
	}
else
	if(argc>=2 && strcmp(argv[1], "-name")==0)
		{
		printf("\n ID_Team S7\n");
		exit(1);
		}
	else 
		for(i=1;i<argc;++i)
			{
			if(!strcmp02(argv[i],"-p"))
				{
				strcpy(archivo,argv[i+1]);
				++i;
				ban_archivo=1;
				}
			else
				if(!strcmp02(argv[i],"-o"))
					{
					strcpy(salida,argv[i+1]);
					++i;
					ban_salida=1;
					}
				else
					if(!strcmp02(argv[i],"-name"))
						{
						ban_name=1;
						}
					else
						if(!strcmp02(argv[i],"-t"))
							{
							max_time=atoi(argv[i+1]);
							++i;
							ban_time=1;
							}
						else
							if(!strcmp02(argv[i],"-s"))
								{//DEBERIA PODER USAR UNA SEMILLA ALEATORIA
								semilla=atoi(argv[i+1]);
								++i;
								if(semilla!=0)
									srand((unsigned)semilla);
								//printf("La semilla fue %ld \n",semilla);
								}
			}
if(ban_archivo && ban_salida)	
	{
	inicio_global = time(NULL);  //Toma el tiempo al inicio
	ban=lee_batch(archivo);
	if(ban)
		{
		ban=lee_defects(archivo);
		if(ban)
			{
			procesa(archivo);
			utiliza_mejor_solucion();
			imprime(salida);
			final_global = time(NULL);
			segundos = difftime(final_global,inicio_global);
			}
		}
	}
return 0;
}
