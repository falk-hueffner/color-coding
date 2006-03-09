#include "graph.h"
using namespace std;

int main(int argc, char* argv[])
{
 
  if(argc!=6)
  {
    cout<<"Anzahl der Paramter nicht korrekt!";
    return 0;
  }
  int path_length=atoi(argv[3]);
  int number_colors=atoi(argv[4]);
  int number_iterations=atoi(argv[5]);
  
  cout<<"Eingegebene Paramter:"<<endl<<"Graph: "<<argv[1]<<endl<<"Startknoten: "<<argv[2]<<endl<<"Pfadlänge: "<<argv[3]<<endl<<"Farbanzahl: "<<argv[4]<<endl<<"Iterationsanzahl: "<<argv[5]<<endl;
  
  long sek;
  time(&sek);
  srand(sek);

  graph protein_network;
  protein_network.read_graph(argv[1]);
  protein_network.read_start_nodes(argv[2]);
  long zeit_start,zeit_ende;
  time(&zeit_start);

  protein_network.compute_results(number_colors,path_length,number_iterations,100);

  time(&zeit_ende);

  protein_network.display_results(100);

  cout<<endl<<"Benötigte Zeit (in Sekunden): "<<(zeit_ende-zeit_start)<<endl;
  return 0;
}
