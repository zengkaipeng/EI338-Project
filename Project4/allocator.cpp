#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

typedef long long ll;
ll MAX;

struct Mem_Block
{
	ll size, start; char *Process_Name;
	Mem_Block *Pre, *Next;
	bool Occupied;
	Mem_Block() 
	{ 
		start = size = 0, Pre = Next = NULL; 
		Occupied = false; Process_Name = NULL; 
	}
	Mem_Block(ll _s, ll _st, char *ps=NULL, bool _O=false, Mem_Block *P=NULL, Mem_Block *N=NULL) 
	{
		size = _s; start = _st; Pre = P; Next = N; Occupied = _O;
		if(ps == NULL) Process_Name = NULL;
		else
		{
			int len = strlen(ps);
			Process_Name = new char[len + 5];
			strcpy(Process_Name, ps);
		}
	}
	~Mem_Block()
	{
		if(Process_Name != NULL)
			delete []Process_Name;
	}
	void Release(){ Occupied = false; }
	void Output(const char *End="")
	{
		printf("Addresses [");
		std::cout << start << ": " << start + size - 1;
		printf("] %s", Occupied ? "Process" : "Unused");
		if(Occupied) printf(" %s", Process_Name);
		printf("%s",End);
	}
}*Fst;

void Remove_Block(Mem_Block *Now)
{
	if(Now->Pre)  Now->Pre->Next = Now->Next;
	if(Now->Next) Now->Next->Pre = Now->Pre;
	delete Now;
}

void Compact(Mem_Block *Now)
{
	if(Now->Occupied) return;
	while(true)
	{
		if(Now->Pre == NULL) break;
		if(Now->Pre->Occupied == false)
			Now = Now->Pre;
		else break;
	}
	
	while(true)
	{
		if(Now->Next == NULL) break;
		if(Now->Next->Occupied == false)
		{
			Now->size += Now->Next->size;
			Remove_Block(Now->Next);
		}
		else break;
	}
}

Mem_Block* Find_Block(ll Size, char Policy)
{
	if(Policy == 'F')
	{
		Mem_Block *Ans = Fst;
		for(; Ans != NULL; Ans = Ans->Next)
			if(Ans->size >= Size && !Ans->Occupied)
				break;
		return Ans;
	}
	if(Policy == 'W' || Policy == 'B')
	{
		Mem_Block *Ans = NULL;
		for(Mem_Block *it = Fst; it != NULL; it = it->Next)
			if(it->size >= Size && !it->Occupied)
			{
				if(Ans == NULL) Ans = it;
				else
				{
					bool fl = Policy == 'W' ?
							  (Ans->size < it->size) :
							  (Ans->size > it->size);
					if(fl) Ans = it;
				}
			}
		return Ans;
	}
	return NULL;
}

int Request_Mem(char *Process_Name, ll Size, char Policy)
{
	Mem_Block *Nb = Find_Block(Size, Policy);
	if(Nb == NULL) return -1;
	
	Mem_Block *Nm = new Mem_Block(
		Size, Nb->start, Process_Name, 
		true, Nb->Pre, Nb
	);
	
	if(Nb->Pre)	Nb->Pre->Next = Nm;
	Nb->Pre = Nm; Nb->size -= Size; 
	Nb->start += Size;
	
	if(Nb == Fst) Fst = Nm;
	if(Nb->size == 0) Remove_Block(Nb);
	return 1;
}

void Release_Mem(char *Process_Name)
{
	for(Mem_Block *it = Fst; it != NULL; it = it->Next)
		if(it->Occupied)
			if(strcmp(it->Process_Name, Process_Name) == 0)
				it->Release();
}

void Output_Mem()
{
	for(Mem_Block *it = Fst; it != NULL; it = it->Next)
		it->Output("\n");
}

int main(int argc, char **argv)
{
	if(argc < 2)
	{
		printf("[INFO] Start Without Initial Information\n");
		return 0;
	}
	MAX = atoll(argv[1]);
	Fst = new Mem_Block(MAX, 0);
	
	printf("allocator>");
	
	char cmd[20], Process_Name[50], Policy;
	ll Req_Size;
	while(true)
	{
		scanf(" %s", cmd);
		if(strcmp(cmd, "X") == 0)
			break;
		if(strcmp(cmd, "C") == 0)
			for(Mem_Block *it = Fst; it; it = it->Next)
				Compact(it);
		if(strcmp(cmd, "STAT") == 0)
			Output_Mem();
		if(strcmp(cmd, "RL") == 0)
		{
			scanf(" %s", Process_Name);
			Release_Mem(Process_Name);
		}
		if(strcmp(cmd, "RQ") == 0)
		{
			scanf(" %s", Process_Name);
			std::cin >> Req_Size;
			scanf(" %c", &Policy);
			int tx = Request_Mem(Process_Name, Req_Size, Policy);
			if(tx == -1)
				printf("[INFO] No Feasible Memory to Allocate\n");
		}
		printf("allocator>");
	}
	delete Fst;
	return 0;
}