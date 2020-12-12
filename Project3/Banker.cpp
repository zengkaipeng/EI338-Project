#include <bits/stdc++.h>
using namespace std;
struct Vec
{
	int len, *num;
	Vec(){ len = 0, num = NULL; }
	Vec(int _l, int *S)
	{
		len = _l; num = new int[_l + 5];
		for(int i = 0; i < len; ++i)
			num[i] = S[i];
	}
	Vec(int _l, int T)
	{
		len = _l; num = new int[_l + 5];
		for(int i = 0; i < len; ++i)
			num[i] = T;
	}
	Vec(const Vec&yu)
	{
		len = yu.len; num = new int[len + 5];
		for(int i = 0; i < len; ++i)
			num[i] = yu.num[i];
	}
	~Vec()
	{
		if(num != NULL)
			delete []num;
	}
	Vec&operator=(const Vec&yu)
	{
		if(num != NULL) delete []num;
		len = yu.len; num = new int[len + 5];
		for(int i = 0; i < len; ++i)
			num[i] = yu.num[i];
		return *this;
	}
	Vec operator+(const Vec&yu)const
	{
		Vec ans = *this;
		for(int i = 0; i < ans.len; ++i)
			ans.num[i] += yu.num[i];
		return ans;
	}
	Vec operator-(const Vec&yu)const
	{
		Vec ans = *this;
		for(int i = 0; i < ans.len; ++i)
			ans.num[i] -= yu.num[i];
		return ans;
	}
	void output(bool sline=false, int width=6)
	{
		char msg[20];
		sprintf(msg, "%c%c%dd", '%', '-', width);
		for(int i = 0; i < len; ++i)
			printf(msg, num[i]);
		if(sline) puts("");
	}
	Vec&operator-=(const Vec&yu)
	{
		for(int i = 0; i < len; ++i)
			num[i] -= yu.num[i];
		return *this;
	}
	Vec&operator+=(const Vec&yu)
	{
		for(int i = 0; i < len; ++i)
			num[i] += yu.num[i];
		return *this;
	}
	void reset(int x=0)
	{
		for(int i = 0; i < len; ++i)
			num[i] = x;
	}
	bool operator<=(const Vec&yu)const
	{
		for(int i = 0; i < len; ++i)
			if(num[i] > yu.num[i])
				return false;
		return true;
	}
};
typedef vector<Vec>::iterator veit;
inline int getint(FILE *f, bool &ef)
{
	char C; int u; bool o(false);
	while((C = fgetc(f)) > 57 || C < 48)
	{
		if(C == EOF)
		{
			ef = true;
			return -1;
		}
		o |= C == '-';
	}
	u = C - '0';
	while((C = fgetc(f)) > 47 && C < 58)
		u += u + (u << 3) + C - 48;
	return o ? -u : u;
}
Vec Available;
vector<Vec> Max, Allocation, Need;
int customer_cnt, resource_cnt;
void release_resource(int customer_num, int release[])
{
	Vec tvec = Vec(resource_cnt, release);
	if(tvec <= Allocation[customer_num])
	{
		Allocation[customer_num] -= tvec;
		Available += tvec;
		Need[customer_num] += tvec;
		puts("[INFO] Resources Released");
	}
	else
		puts("[INFO] Not So Much Resources to Release");
}
inline bool Check_Safety()
{
	bool *flag = new bool[customer_cnt + 5], tfind;
	int *Stack = new int[customer_cnt + 5];
	Stack[0] = 0;
	memset(flag, 0, sizeof(bool) * (customer_cnt + 5));
	for(int i = 0; i < customer_cnt; ++i)
	{
		tfind = false;
		for(int j = 0; j < customer_cnt; ++j)
			if(!flag[j] && Need[j] <= Available)
			{
				Available += Allocation[j];
				Stack[++Stack[0]] = j;
				tfind = flag[j] = true;
				break;
			}
		if(!tfind) break;
	}
	for(; Stack[0]; --Stack[0])
		Available -= Allocation[Stack[Stack[0]]];
	delete []flag;
	delete []Stack;
	return tfind;
}
inline int request_resource(int customer_num, int request[])
{
	Vec tvec = Vec(resource_cnt, request);
	if(tvec <= Need[customer_num])
	{
		Allocation[customer_num] += tvec;
		Need[customer_num] -= tvec;
		Available -= tvec;
		if(Check_Safety()) return 0;
		else
		{
			Allocation[customer_num] -= tvec;
			Need[customer_num] += tvec;
			Available += tvec;
		}
	}
	return -1;
}
int main(int argc, char *argv[])
{
	printf("[INFO] Initializing...\n");
	int *nums = new int[argc + 1];
	for(int i = 1; i < argc; ++i)
		nums[i - 1] = atoi(argv[i]);
	
	Available = Vec(argc - 1, nums);
	resource_cnt = argc - 1;
	
	printf("[INFO] Generating Max Matrix...\n");
	FILE * fmax = fopen("Max.txt", "r");
	for(int p, tc(0); true;)
	{
		bool fl = false;
		p = getint(fmax, fl);
		if(fl) break;
		nums[tc++] = p;
		if(tc == argc - 1)
		{
			Max.push_back(Vec(tc, nums));
			Need.push_back(Vec(tc, nums));
			Allocation.push_back(Vec(tc, 0));
			tc = 0;
		}
	}
	fclose(fmax);
	customer_cnt = Max.size();
	printf("[INFO] Done\n");
	
	putchar('>');
	char cmd[10];
	for(int idx; ~scanf(" %s", cmd);)
	{
		if(strcmp(cmd, "*") == 0)
		{
			puts("Available\n-------------------");
			Available.output();
			puts("");
			puts("Maximun\n-------------------");
			for(veit it = Max.begin(); it != Max.end(); ++it)
				it -> output(true);
			puts("");
			puts("Allocation\n-------------------");
			for(veit it = Allocation.begin(); it != Allocation.end(); ++it)
				it -> output(true);
			puts("");
			puts("Need\n-------------------");
			for(veit it = Need.begin(); it != Need.end(); ++it)
				it -> output(true);
			puts("");
		}
		else
		{
			scanf("%d", &idx);
			for(int i = 0; i < argc - 1; ++i)
				scanf("%d", nums + i);
			if(strcmp(cmd, "RQ") == 0)
			{
				int tx = request_resource(idx, nums);
				if(tx == -1)
					puts("[INFO] Fail to Grant the Request");
				if(tx == 0)
					puts("[INFO] The Request is Granted");
			}
			else if(strcmp(cmd, "RL") == 0)
				release_resource(idx, nums);
		}
		putchar('>');
	}
	delete []nums;
	return 0;
}