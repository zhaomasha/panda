#include "panda_types.hpp"
#include "panda_subgraph.hpp"
int main()
{
	Subgraph s;
	s.init("sd");
	s.add_file();
	s.format(1024);
	s.add_file();
	s.update_index();
}
