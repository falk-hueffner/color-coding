#include "graph.h"
using namespace std;

long ins[]={1,1<<1,1<<2,1<<3,1<<4,1<<5,1<<6,1<<7,1<<8,1<<9,1<<10,1<<11,1<<12,1<<13,1<<14,1<<15};
long del[]={~1,~(1<<1),~(1<<2),~(1<<3),~(1<<4),~(1<<5),~(1<<6),~(1<<7),~(1<<8),~(1<<9),~(1<<10),~(1<<11),~(1<<12),~(1<<13),~(1<<14),~(1<<15)};

//--------------------------------------------------

void graph::read_graph(char* filename)
{
  FILE *datei;
  int i,j,n1i,n2i;
  char n1[10],n2[10];
  string n1s,n2s;
  float w;

  number_nodes=0;
  number_neighbours.clear();
  colors.clear();
  node_list1.clear();
  node_list2.clear();
  neighbours_list.clear();
  n_weights_list.clear();


  datei = fopen(filename, "r");
  if (!datei) {
    printf("Error while reading graph-file!");
    getchar();
    return;
  }

  cout<<endl<<endl<<"Eingelesene Graph-Datei:"<<endl;
  i=0;
  while (feof(datei)==0)
  {
    fscanf(datei, "%s %s %f", n1,n2,&w);
    //printf("%s\t%s\t%f\n", n1,n2,w);

    n1s=string(n1);
    n2s=string(n2);
    
    if(node_list1.count(n1s)==0)
    {
	node_list1.insert(PAIR_STR_INT(n1s,i));
	node_list2.insert(PAIR_INT_STR(i,n1s));
	neighbours_list.push_back(VEC_INT());
	n_weights_list.push_back(VEC_FLT());
	n1i=i;
	i++;
    } 
    else n1i=node_list1.find(n1s)->second;

    if(node_list1.count(n2s)==0)
    {
	node_list1.insert(PAIR_STR_INT(n2s,i));
	node_list2.insert(PAIR_INT_STR(i,n2s));
	neighbours_list.push_back(VEC_INT());
	n_weights_list.push_back(VEC_FLT());
	n2i=i;
	i++;
    } 
    else n2i=node_list1.find(n2s)->second;

    (neighbours_list[n1i]).push_back(n2i);
    (neighbours_list[n2i]).push_back(n1i);
    (n_weights_list[n1i]).push_back(-log(w));
    (n_weights_list[n2i]).push_back(-log(w));

  }

  fclose(datei);
  
  j=0;
  number_nodes=node_list1.size();
  number_neighbours=VEC_INT(number_nodes,0);
  colors=VEC_INT(number_nodes,0);
  for(i=0;i<node_list1.size();i++)
  {
    number_neighbours[i]=(neighbours_list[i]).size();
    j+=number_neighbours[i];
  }
  cout<<"Number of nodes: "<<number_nodes<<endl<<"Number of edges: "<<j/2<<endl;
}

//--------------------------------------------------

void graph::read_start_nodes(char* filename)
{
  char n1[10];
  int n1i;
  string n1s;
  VEC_STR start_vec;
  
  FILE* datei;
  
  start_nodes.clear();

  datei = fopen(filename, "r");
  if (!datei) {
    printf("Error while reading start-node-file!");
    getchar();
    return;
  }

  cout<<endl<<endl<<"Startknoten:  ";
  while (feof(datei)==0)
  {
    fscanf(datei, "%s", n1);
  
    n1s=string(n1);
    if(node_list1.count(n1s)==1)
    {
      cout<<n1s<<"  ";
      n1i=node_list1.find(n1s)->second;
      start_nodes.push_back(n1i);
    }
  }
  fclose(datei);

}

//--------------------------------------------------

void graph::color_nodes(int number_colors)
{
  int i;
  for(i=0;i<number_nodes;i++)
  {
    colors[i]=rand()%number_colors;
  }
}

//--------------------------------------------------

PAIR_FLT_VIN graph::search_path(int path_length,float weight_border)
{
  MAP_PLI_PIF paths[path_length];
  MAP_PLI_PIF_ITER listpos,listpos2;
  
  MAP_FLT_PLI result;
  MAP_FLT_PLI_ITER respos;
  
  PAIR_LNG_INT entry;
  
  int i,j,node,nn,actn;
  float weight;
  for(i=0;i<start_nodes.size();i++)
  {
    paths[0].insert(PAIR_PLI_PIF(PAIR_LNG_INT(ins[colors[start_nodes[i]]],start_nodes[i]),PAIR_INT_FLT(start_nodes[i],0.0)));
  }

  for(i=0;i<path_length-1;i++)
  {
    //cout<<"i: "<<i<<endl;
    for(listpos=paths[i].begin();listpos!=paths[i].end();listpos++)
    {
      node=(listpos->first).second;
      nn=number_neighbours[node];
      //cout<<"node: "<<node<<"  anz nachbarn:"<<nn<<endl;
      for(j=0;j<nn;j++)
      {
	actn=neighbours_list[node][j];
	if((ins[colors[actn]]&((listpos->first).first))!=0) continue;
	entry=PAIR_LNG_INT(ins[colors[actn]]|((listpos->first).first),actn);
	weight=n_weights_list[node][j]+((listpos->second).second);
	if (weight>weight_border) continue;
	if(paths[i+1].count(entry)==0)
	{
	  paths[i+1].insert(PAIR_PLI_PIF(entry,PAIR_INT_FLT(node,weight)));
	}
	else
	{
	  listpos2=paths[i+1].find(entry);
	  if(((listpos2->second).second)>weight) (listpos2->second)=PAIR_INT_FLT(node,weight);
	}
      }
    }
  }

  for(listpos=paths[path_length-1].begin();listpos!=paths[path_length-1].end();listpos++)
  {
    result.insert(PAIR_FLT_PLI((listpos->second).second,listpos->first));
  }

  VEC_INT res_path(path_length,0);
  PAIR_FLT_VIN compl_res;

  if(result.begin()!=result.end())
  {
    long act_color=(result.begin()->second).first;
    int act_node=(result.begin()->second).second;
    int last_node=((paths[path_length-1].find(result.begin()->second))->second).first;

    res_path[path_length-1]=act_node;
  
    for(i=path_length-2;i>=0;i--)
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

//--------------------------------------------------

PAIR_FLT_VIN graph::search_path_array(int path_length,int number_colors,float weight_border)
{
  
  MAP_FLT_PLI result;
  MAP_FLT_PLI_ITER respos;
  
  PAIR_LNG_INT entry;
  stack<PAIR_LNG_INT> nodes_to_do[2];

  int i,j,node,nn,act_node,color,new_color;
  bool act_stack=false;
  float weight;
  
//   array_weights.assign((int)pow(2,number_colors),VEC_FLT(number_nodes,INT_MAX));
//   array_last_nodes.assign((int)pow(2,number_colors),VEC_INT(number_nodes,INT_MAX));
  

  for(i=0;i<start_nodes.size();i++)
  {
    color=ins[colors[start_nodes[i]]];
    act_node=start_nodes[i];
    array_weights[color][act_node]=0;
    nodes_to_do[0].push(PAIR_LNG_INT(color,act_node));
  }

  for(i=0;i<path_length-1;i++)
  {
    while(!nodes_to_do[act_stack].empty())
    {
      color=(nodes_to_do[act_stack].top()).first;
      node=(nodes_to_do[act_stack].top()).second;
      nodes_to_do[act_stack].pop();
      nn=number_neighbours[node];
      for(j=0;j<nn;j++)
      {
	act_node=neighbours_list[node][j];
	if((ins[colors[act_node]]&color)!=0) continue;
	new_color=ins[colors[act_node]]|color;
        entry=PAIR_LNG_INT(new_color,act_node);
        weight=n_weights_list[node][j]+array_weights[color][node];
	if (weight>weight_border) continue;

	if(array_weights[new_color][act_node]>weight)
	{
	  array_weights[new_color][act_node]=weight;
	  array_last_nodes[new_color][act_node]=node;
	  nodes_to_do[!act_stack].push(entry);
	}
      }
    }
    act_stack=!act_stack;
  }

  while(!nodes_to_do[act_stack].empty())
  {
    color=(nodes_to_do[act_stack].top()).first;
    node=(nodes_to_do[act_stack].top()).second;
    
    result.insert(PAIR_FLT_PLI(array_weights[color][node],nodes_to_do[act_stack].top()));
    nodes_to_do[act_stack].pop();
  }

  VEC_INT res_path(path_length,0);
  PAIR_FLT_VIN compl_res;

  if(result.begin()!=result.end())
  {
    long act_color=(result.begin()->second).first;
    int act_node=(result.begin()->second).second;
    int last_node=array_last_nodes[act_color][act_node];

    res_path[path_length-1]=act_node;
  
    for(i=path_length-2;i>=0;i--)
    {
      res_path[i]=last_node;
      last_node=array_last_nodes[act_color &= del[colors[act_node]]][last_node];
      act_node=res_path[i];
    }
    compl_res=PAIR_FLT_VIN(result.begin()->first,res_path);
  }
  else
  {
    compl_res=PAIR_FLT_VIN(INT_MAX,res_path);
  }

  return compl_res ;
}

//--------------------------------------------------

void graph::compute_results(int number_colors, int path_length, int number_iterations, int number_results)
{
  int i,j;
  
  PAIR_FLT_VIN res;
  //MAP_FLT_VIN_ITER results_pos;
  
  float weight_border=INT_MAX;
  results.clear();

  // array_weights=new float[(int)pow(2,number_colors)][number_nodes];
  
  //array_weights.assign((int)pow(2,number_colors),VEC_FLT(number_nodes,INT_MAX));
  //array_last_nodes.assign((int)pow(2,number_colors),VEC_INT(number_nodes,INT_MAX));

  for(i=0;i<number_iterations;i++)
  {
    color_nodes(number_colors);
    res=search_path(path_length,weight_border);
    //res=search_path_array(path_length,number_colors,weight_border);
    results.insert(res);
  }
}

//--------------------------------------------------

void graph::display_results(int number_results)
{
  MAP_FLT_VIN_ITER result_pos;
  int i,j;

  cout<<endl<<"Best "<<number_results<<" of "<<results.size()<<" found paths:"<<endl;
  result_pos=results.begin();
  for(j=0;((result_pos!=results.end())&&(j<number_results));result_pos++,j++)
  {
    cout<<endl<<result_pos->first<<"\t  ";
    for(i=0;i<(result_pos->second).size();i++)
    {
      cout<<node_list2.find((result_pos->second)[i])->second<<" ";
    }
  }
}
