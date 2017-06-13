#include "Util.hpp"
#include <vector>
#include <map>
#include <climits>
#include <iostream>

int MinMem(int blk_dimi, int blk_dimj, int blk_diml){
  return (blk_dimi*blk_diml+blk_dimj*blk_diml+blk_dimi*blk_dimj);
}

struct BlkUse{
  int blk;
  int prev;
  int next;
};

unsigned long long SpillCase1(int blk_size, int num_blk, int num_blk_mem){
  unsigned long long spill = 0;
  for(int group=0; group<num_blk; group++){
    vector<int> prev_blk_use(num_blk,-1);
    vector<BlkUse> use_list;
    map<int,map<int,int>> u_b_index;

    for(int u=0; u<num_blk; u++){
      for(int b=0; b<min(group, u)+1; b++){
        int prev = prev_blk_use[b];
	use_list.push_back({b,prev,-1});
	if(prev != -1){
	  use_list[prev].next = use_list.size()-1;
	}
	prev_blk_use[b] = use_list.size()-1;
	u_b_index[u][b] = use_list.size()-1;
      }
    }

    //for each group
    for(int u=0; u<num_blk; u++){
      if(u <= group){
        int num_b = min(group,u)+1;
	if(num_b > num_blk_mem){
	  //add new eviction
	  int use = u_b_index[u][num_b-1];
	  while(use_list[use-1].next != -1){
	    spill += blk_size;
	    use = use_list[use-1].next;
	  }
	}
      }
    }
  }
  return spill;
}

unsigned long long SpillCase2(int blk_size, int num_blk, int num_blk_mem){
  unsigned long long spill = 0;
  vector<BlkUse> use_list;
  vector<int> prev_blk_use(num_blk*num_blk,-1);
  map<int,map<int,int>> u_b_index;

  for(int u=0; u<num_blk; u++){
    for(int bcol=0; bcol<num_blk; bcol++){
      for(int b=0; b<num_blk; b++){
        if(b < (min(u,bcol)+1)){
	  int b_idx = bcol*num_blk+b;
	  u_b_index[u][b_idx] = use_list.size();
	  int prev = prev_blk_use[b_idx];
	  if(prev != -1){
	    use_list[prev].next = use_list.size();
	  }
	  prev_blk_use[b_idx] = use_list.size();
	  use_list.push_back({b_idx, prev, -1});
	}
      }
    }
  }

  int mem_used = 0;
  for(int u=0; u<num_blk; u++){
    //checl all newly added block
    for(int bcol=0; bcol<num_blk; bcol++){
      for(int b=0; b<num_blk; b++){
        if((bcol >= u) && (b == u)){
	  //this is newly added block
	  int b_idx = bcol*num_blk+b;
	  if(mem_used == num_blk_mem){
	    int use = u_b_index[u][b_idx];
	    while(use_list[use-1].next != -1){
	      spill += blk_size;
	      use = use_list[use-1].next;
	    }
	  }
	  else{
	    mem_used++;
	  }
	}
      }
    }
  }
  return spill;
}

unsigned long long SpillCase3(int blk_size, int num_blk, int num_blk_mem){
  unsigned long long spill = 0;
  for(int group=0; group<num_blk; group++){
    vector<int> prev_blk_use(num_blk, -1);
    vector<BlkUse> use_list;
    map<int,map<int,int>> u_b_index;

    for(int u=0; u<group+1; u++){
      for(int b=0; b<num_blk; b++){
        if(b >= u){
	  int prev = prev_blk_use[b];
	  use_list.push_back({b,prev,-1});
	  if(prev != -1){
	    use_list[prev].next = use_list.size()-1;
	  }
	  prev_blk_use[b] = use_list.size()-1;
	  u_b_index[u][b] = use_list.size()-1;
	}
      }
    }

    //for each group
    int u=0;
    for(int b=0; b<num_blk; b++){
      if(b+1 > num_blk_mem){
        int use = u_b_index[u][b];
	while(use_list[use-1].next != -1){
	  spill += blk_size;
	  use = use_list[use-1].next;
	}
      }
    }
  }
  return spill;
}

unsigned long long SpillCase4(int blk_size, int num_blk, int num_blk_mem){
  unsigned long long spill = 0;
  vector<BlkUse> use_list;
  vector<int> prev_blk_use(num_blk*num_blk,-1);
  map<int,map<int,int>> u_b_index;

  for(int u=0; u<num_blk; u++){
    for(int bcol=0; bcol<u+1; bcol++){
      for(int b=0; b<num_blk; b++){
        if(b < (num_blk-bcol)){
	  int b_idx = bcol*num_blk+b;
	  u_b_index[u][b_idx] = use_list.size();
	  int prev = prev_blk_use[b_idx];
	  if(prev != -1){
	    use_list[prev].next = use_list.size();
	  }
	  prev_blk_use[b_idx] = use_list.size();
	  use_list.push_back({b_idx,prev,-1});
	}
      }
    }
  }

  int mem_used = 0;
  for(int u=0; u<num_blk; u++){
    int bcol = u;
    for(int b=0; b<num_blk; b++){
      if(b < (num_blk-bcol)){
        int b_idx = bcol*num_blk+b;
	if(mem_used == num_blk_mem){
	  int use = u_b_index[u][b_idx];
	  while(use_list[use-1].next != -1){
	    spill += blk_size;
	    use = use_list[use-1].next;
	  }
	}
	else{
	  mem_used++;
	}
      }
    }
  }
  return spill;
}

unsigned long long SpillCase5(int blk_size, int num_blk, int num_blk_mem){
  unsigned long long spill = 0;
  for(int group=0; group<num_blk; group++){
    vector<int> prev_blk_use(num_blk,-1);
    vector<BlkUse> use_list;
    map<int,map<int,int>> u_b_index;

    for(int u=0; u<(num_blk-group); u++){
      for(int b=0; b<num_blk; b++){
        if(b < (num_blk-group)){
	  int prev = prev_blk_use[b];
	  use_list.push_back({b,prev,-1});
	  if(prev != -1){
	    use_list[prev].next = use_list.size()-1;
	  }
	  prev_blk_use[b] = use_list.size()-1;
	  u_b_index[u][b] = use_list.size()-1;
	}
      }
    }

    //for each group
    int u=0;
    for(int b=0; b<(num_blk-group); b++){
      if(b+1 > num_blk_mem){
        int use = u_b_index[u][b];
	while(use_list[use-1].next != -1){
	  spill += blk_size;
	  use = use_list[use-1].next;
	}
      }
    }
  }
  return spill;
}

unsigned long long SpillCase6(int blk_size, int num_blk, int num_blk_mem){
  unsigned long long spill = 0;
  vector<BlkUse> use_list;
  vector<int> prev_blk_use(num_blk*num_blk,-1);
  map<int,map<int,int>> u_b_index;

  for(int u=0; u<num_blk; u++){
    for(int bcol=u; bcol<num_blk; bcol++){
      for(int b=0; b<num_blk; b++){
        if(b >= u){
	  int b_idx = bcol*num_blk+b;
	  u_b_index[u][b_idx] = use_list.size();
	  int prev = prev_blk_use[b_idx];
	  if(prev != -1){
	    use_list[prev].next = use_list.size();
	  }
	  prev_blk_use[b_idx] = use_list.size();
	  use_list.push_back({b_idx,prev,-1});
	}
      }
    }
  }

  int mem_used = 0;
  int u = 0;
  //checl all newly added block
  for(int bcol=0; bcol<num_blk; bcol++){
    for(int b=0; b<num_blk; b++){
      int b_idx = bcol*num_blk+b;
      int use = u_b_index[u][b_idx];
      if(mem_used == num_blk_mem){
        while(use_list[use-1].next != -1){
	  spill += blk_size;
	  use = use_list[use-1].next;
	}
      }
      else{
        if(use_list[use].next != -1){
	  mem_used++;
	}
      }
    }
  }
  return spill;
}
