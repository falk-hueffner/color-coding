#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <stdio.h>
#include <time.h>
#include <math.h>
using namespace std;

typedef vector<int> VEC_INT;
typedef vector<VEC_INT> VEC_VEC_INT;
typedef vector<float> VEC_FLT;
typedef vector<VEC_FLT> VEC_VEC_FLT;
typedef map<string, int> MAP_STR_INT;
typedef MAP_STR_INT::iterator MAP_STR_INT_ITER;
typedef pair<string, int> PAIR_STR_INT;

typedef vector<string> VEC_STR;

typedef map<int, string> MAP_INT_STR;
typedef MAP_INT_STR::iterator MAP_INT_STR_ITER;
typedef pair<int, string> PAIR_INT_STR;

typedef pair<long, int> PAIR_LNG_INT;
typedef pair<int,float> PAIR_INT_FLT;
typedef pair<PAIR_LNG_INT,PAIR_INT_FLT> PAIR_PLI_PIF;
typedef map<PAIR_LNG_INT,PAIR_INT_FLT> MAP_PLI_PIF;
typedef MAP_PLI_PIF::iterator MAP_PLI_PIF_ITER;

typedef pair<float,PAIR_LNG_INT> PAIR_FLT_PLI;
typedef map<float,PAIR_LNG_INT> MAP_FLT_PLI;
typedef MAP_FLT_PLI::iterator MAP_FLT_PLI_ITER;

typedef pair<float,VEC_INT> PAIR_FLT_VIN;
typedef map<float,VEC_INT> MAP_FLT_VIN;
typedef MAP_FLT_VIN::iterator MAP_FLT_VIN_ITER;

long ins[]={1,1<<1,1<<2,1<<3,1<<4,1<<5,1<<6,1<<7,1<<8,1<<9,1<<10,1<<11,1<<12,1<<13,1<<14,1<<15};
long del[]={~1,~(1<<1),~(1<<2),~(1<<3),~(1<<4),~(1<<5),~(1<<6),~(1<<7),~(1<<8),~(1<<9),~(1<<10),~(1<<11),~(1<<12),~(1<<13),~(1<<14),~(1<<15)};

void color_nodes(int* colors,int number_nodes,int cnumbers);
PAIR_FLT_VIN search_path(int number_nodes,int* number_neighbours,int* colors,int** neighbours,float** weights,int snnumber, int* start_nodes, int plength);
int randint(int max);

int main(int argc, char* argv[])
{
  
  MAP_STR_INT node_list1;
  MAP_STR_INT_ITER pos;
  VEC_VEC_INT nb_vec;
  VEC_VEC_FLT we_vec;

  MAP_INT_STR node_list2;
  MAP_INT_STR_ITER kl2_pos;

  FILE *datei;
  
  if(argc!=6)
  {
    cout<<"Anzahl der Paramter nicht korrekt!";
    return 0;
  }
  int path_length=atoi(argv[3]);
  int number_colors=atoi(argv[4]);
  int number_repeats=atoi(argv[5]);
  
  cout<<"Eingegebene Paramter:"<<endl<<"Graph: "<<argv[1]<<endl<<"Startknoten: "<<argv[2]<<endl<<"Pfadlänge: "<<argv[3]<<endl<<"Farbanzahl: "<<argv[4]<<endl<<"Iterationsanzahl: "<<argv[5]<<endl;
  
  long sek;
  time(&sek);
  srand(sek);

  datei = fopen(argv[1], "r");
  if (!datei) {
    printf("Fehler!");
    getchar();
    return 0;
  }
  
  int i=0,j,k1i,k2i,vec_size;
  char k1[10],k2[10];
  string k1s,k2s;
  float z3;
  
  cout<<endl<<"Eingelesene Graph-Datei:"<<endl;
  while (feof(datei)==0)
  {
    fscanf(datei, "%s %s %f", k1,k2,&z3);
    printf("%s\t%s\t%f\n", k1,k2,z3);

    k1s=string(k1);
    k2s=string(k2);

    if(node_list1.count(k1s)==0)
    {
	node_list1.insert(PAIR_STR_INT(k1s,i));
	node_list2.insert(PAIR_INT_STR(i,k1s));
	nb_vec.push_back(VEC_INT());
	we_vec.push_back(VEC_FLT());
	k1i=i;
	i++;
    } 
    else k1i=node_list1.find(k1s)->second;

    if(node_list1.count(k2s)==0)
    {
	node_list1.insert(PAIR_STR_INT(k2s,i));
	node_list2.insert(PAIR_INT_STR(i,k2s));
	nb_vec.push_back(VEC_INT());
	we_vec.push_back(VEC_FLT());
	k2i=i;
	i++;
    } 
    else k2i=node_list1.find(k2s)->second;

    (nb_vec[k1i]).push_back(k2i);
    (nb_vec[k2i]).push_back(k1i);
    (we_vec[k1i]).push_back(-log(z3));
    (we_vec[k2i]).push_back(-log(z3));

  }
  fclose(datei);

  datei = fopen(argv[2], "r");
  if (!datei) {
    printf("Fehler!");
    getchar();
    return 0;
  }

  VEC_STR start_vec;
  
  while (feof(datei)==0)
  {
    fscanf(datei, "%s", k1);
    k1s=string(k1);
    start_vec.push_back(k1s);
  }
  fclose(datei);

  cout<<endl<<endl<<"Startknoten:  ";
  int start_nodes[start_vec.size()];
  for(j=0;j<start_vec.size();j++)
  {
    start_nodes[j]=(node_list1.find(start_vec[j]))->second;
    cout<<start_vec[j]<<"  ";
  }
  
  int number_nodes=i;
  int number_neighbours[node_list1.size()];
  int colors[node_list1.size()];
  int* neighbours[node_list1.size()];
  float* weights[node_list1.size()];

  cout<<endl<<endl<<"Interne Zuordnung der Knoten zu Zahlen:"<<endl;
  for(pos=node_list1.begin(),i=0;pos!=node_list1.end();pos++,i++)
  {
    cout<<pos->first<<"\t"<<pos->second<<"\t"<<endl;
    vec_size=nb_vec[i].size();
    number_neighbours[i]=vec_size;

    int *ne=new int[vec_size];
    float *we=new float[vec_size];
    
    for(j=0;j<vec_size;j++)
    {
      ne[j]=nb_vec[i][j];
      we[j]=we_vec[i][j];
    }
    neighbours[i]=ne;
    weights[i]=we;
  }
  
  cout<<endl<<endl<<"Nachbarschaften:  "<<endl;
  for(i=0;i<number_nodes;i++)
  {
    cout<<node_list2.find(i)->second<<" -> "<<"\t";
    for(j=0;j<number_neighbours[i];j++)
    {
      cout<<node_list2.find(neighbours[i][j])->second<<"  "<<weights[i][j]<<"\t";
    }
    cout<<endl;
  }

 
  PAIR_FLT_VIN res;

  MAP_FLT_VIN results;
  MAP_FLT_VIN_ITER result_pos;
 
  for(i=0;i<number_repeats;i++)
  {
    color_nodes(colors,number_nodes,number_colors);
    res=search_path(number_nodes,number_neighbours,colors,neighbours,weights,2,start_nodes,path_length);
    results.insert(res);
  }

  cout<<endl<<"Gefundene Pfade: ";
  for(result_pos=results.begin();result_pos!=results.end();result_pos++)
  {
    cout<<endl<<result_pos->first<<"\t  ";
    for(i=0;i<(result_pos->second).size();i++)
    {
      cout<<node_list2.find((result_pos->second)[i])->second<<" ";
    }
  }
  return 0;
}

void color_nodes(int* colors,int number_nodes,int cnumber)
{
//   long sek;
//   int i;
//   sek=time(NULL);
//   srand((unsigned) sek);
//   cout<<rand()<<endl;
  int i;
  for(i=0;i<number_nodes;i++)
  {
    colors[i]=rand()%cnumber;
  }
}

PAIR_FLT_VIN search_path(int number_nodes,int* number_neighbours,int* colors,int** neighbours,float** weights,int snnumber, int* start_nodes, int plength)
{
  MAP_PLI_PIF paths[plength];
  MAP_PLI_PIF_ITER listpos,listpos2;
  
  MAP_FLT_PLI result;
  MAP_FLT_PLI_ITER respos;
  
  PAIR_LNG_INT* entry;
  
  int i,j,node,nn,actn;
  float weight;
  for(i=0;i<snnumber;i++)
  {
    paths[0].insert(PAIR_PLI_PIF(PAIR_LNG_INT(ins[colors[start_nodes[i]]],start_nodes[i]),PAIR_INT_FLT(start_nodes[i],0.0)));
  }

  for(i=0;i<plength-1;i++)
  {
    //cout<<"i: "<<i<<endl;
    for(listpos=paths[i].begin();listpos!=paths[i].end();listpos++)
    {
      node=(listpos->first).second;
      nn=number_neighbours[node];
      //cout<<"node: "<<node<<"  anz nachbarn:"<<nn<<endl;
      for(j=0;j<nn;j++)
      {
	actn=neighbours[node][j];
	if((ins[colors[actn]]&((listpos->first).first))!=0) continue;
	entry=new PAIR_LNG_INT(ins[colors[actn]]|((listpos->first).first),actn);
	weight=weights[node][j]+((listpos->second).second);
	if(paths[i+1].count(*entry)==0)
	//if(paths[i+1].count(PAIR_LNG_INT(ins[colors[actn]]|((listpos->first).first),actn))==0)
	{
	  paths[i+1].insert(PAIR_PLI_PIF(*entry,PAIR_INT_FLT(node,weight)));
	  //paths[i+1].insert(PAIR_PLI_PIF(PAIR_LNG_INT(ins[colors[actn]]|((listpos->first).first),actn),PAIR_INT_FLT(node,weight)));

	}
	else
	{
	  listpos2=paths[i+1].find(*entry);
	  //listpos2=paths[i+1].find(PAIR_LNG_INT(ins[colors[actn]]|((listpos->first).first),actn));
	  if(((listpos2->second).second)>weight) (listpos2->second)=PAIR_INT_FLT(node,weight);
	}
      }
    }
  }

  for(listpos=paths[plength-1].begin();listpos!=paths[plength-1].end();listpos++)
  {
    result.insert(PAIR_FLT_PLI((listpos->second).second,listpos->first));
  }

//    cout<<i<<endl;
//    cout<<paths[0].size()<<"  "<<paths[1].size()<<"  "<<paths[2].size()<<"  "<<paths[3].size()<<"  "<<paths[4].size()<<"  "<<endl;
//    cout<<result.begin()->first<<"   "<<((result.begin()->second).second)<<endl;
  
  VEC_INT res_path(plength,0);
  PAIR_FLT_VIN compl_res;

  if(result.begin()!=result.end())
  {
    long act_color=(result.begin()->second).first;
    int act_node=(result.begin()->second).second;
    int last_node=((paths[plength-1].find(result.begin()->second))->second).first;
    //cout<<paths[plength-1].count(result.begin()->second)<<"  ";

    res_path[plength-1]=act_node;
  
    for(i=plength-2;i>=0;i--)
    {
      listpos=paths[i].find(PAIR_LNG_INT(act_color &= del[colors[act_node]],last_node));
      act_node=last_node;
      last_node=(listpos->second).first;
      res_path[i]=act_node;
    }
    compl_res=PAIR_FLT_VIN(result.begin()->first,res_path);
  }
  else
  {
    compl_res=PAIR_FLT_VIN(INT_MAX,res_path);
  }
  return compl_res ;
}

int randint(int max)
{
  long sek;
  time(&sek);
  srand((unsigned) sek);
  
  return rand()%max;
}
