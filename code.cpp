#include<iostream>
#include<vector>
#include<tuple>
#include<set>
#include<cstdlib>
#include<cctype>
using namespace std;
const string NOT="~",AND="&",OR="|",COND="->";
const string FORALL="A_",EXISTS="E_";
enum RULE{A,MPP,MTT,DN,CP,AI,AE,OI,OE,RAA,UI,UE,EI,EE,flip};
struct formula{
	const formula *l,*r;
	string node;
	formula():l(nullptr),r(nullptr),node("__undefined__"){}
	formula(const string s):l(nullptr),r(nullptr),node(s){}
	formula(const formula*l,const formula*r,const string s):l(l),r(r),node(s){}
	bool eq(const formula*rhs)const
	{
		if(node!=rhs->node)return false;
		if(l!=nullptr)
		{
			if(rhs->l==nullptr||!l->eq(rhs->l))return false;
		}
		else if(rhs->l!=nullptr)return false;
		if(r!=nullptr)
		{
			if(rhs->r==nullptr||!r->eq(rhs->r))return false;
		}
		else if(rhs->r!=nullptr)return false;
		return true;
	}
	string to_s()const
	{
		if(l==nullptr)
		{
			if(r==nullptr)return node;
			else return node+r->to_s();
		}
		else if(node==FORALL||node==EXISTS)
		{
			return "("+node+l->to_s()+"."+r->to_s()+")";
		}
		else//if(node==AND||node==OR||node==COND)
		{
			return "("+l->to_s()+node+r->to_s()+")";
		}
	}
};
const formula*ERROR=new formula();

/*
 * formula	:= pred | pred term | NOT formula | "(" formula [AND, OR, COND] formula ")" | "(" [FORALL, EXISTS] term "." formula ")"
 * NOT		:= "~"
 * AND		:= "&"
 * OR		:= "|"
 * COND		:= "->"
 * FORALL	:= "A_"
 * EXISTS	:= "E_"
 * pred		:= [A-Z]+
 * term		:= [a-z]+
 */
bool isterm(const formula*siki)
{
	if(siki==nullptr||siki==ERROR)return false;
	for(char c:siki->node)if(!islower(c))return false;
	return siki->l==nullptr&&siki->r==nullptr;
}
bool ispred(const formula*siki)
{
	if(siki==nullptr||siki==ERROR)return false;
	for(char c:siki->node)if(!isupper(c))return false;
	return siki->l==nullptr&&(siki->r==nullptr||isterm(siki->r));
}
bool has_term(const formula*siki,const string&term)
{
	if(siki==nullptr)return false;
	return siki->node==term||has_term(siki->l,term)||has_term(siki->r,term);
}
const formula*meidai(int&id,const string&s)
{
	if(s[id]=='(')
	{
		id++;
		{//FORALL, EXISTS
			string node="";
			for(const string&sym:{FORALL,EXISTS})
			{
				if(s.substr(id,sym.size())==sym)
				{
					node=sym;
					id+=sym.size();
					break;
				}
			}
			if(node!="")
			{
				const formula*l=meidai(id,s);
				if(l==ERROR||!isterm(l))return ERROR;
				if(s[id]!='.')return ERROR;
				id++;
				const formula*r=meidai(id,s);
				if(r==ERROR||!has_term(r,l->node))return ERROR;
				if(s[id]!=')')return ERROR;
				id++;
				const formula*now=new formula(l,r,node);
				return now;
			}
		}
		{//AND, OR, COND
			const formula*l=meidai(id,s);
			if(l==ERROR)return ERROR;
			string node="";
			for(const string&sym:{AND,OR,COND})
			{
				if(s.substr(id,sym.size())==sym)
				{
					node=sym;
					id+=sym.size();
					break;
				}
			}
			if(node!="")
			{
				const formula*r=meidai(id,s);
				if(r==ERROR)return ERROR;
				if(s[id]!=')')return ERROR;
				id++;
				const formula*now=new formula(l,r,node);
				return now;
			}
		}
		return ERROR;
	}
	else if(isupper(s[id]))
	{
		string node="";
		while(isupper(s[id]))node+=s[id++];
		if(id<s.size()&&islower(s[id]))
		{
			const formula*r=meidai(id,s);
			if(r==ERROR||!isterm(r))return ERROR;
			const formula*now=new formula(nullptr,r,node);
			return now;
		}
		else
		{
			const formula*now=new formula(node);
			return now;
		}
	}
	else if(islower(s[id]))
	{
		string node="";
		while(islower(s[id]))node+=s[id++];
		const formula*now=new formula(node);
		return now;
	}
	else if(s.substr(id,NOT.size())==NOT)
	{
		id+=NOT.size();
		const formula*r=meidai(id,s);
		if(r==ERROR)return ERROR;
		const formula*now=new formula(nullptr,r,NOT);
		return now;
	}
	else return ERROR;
}
pair<set<string>,bool>term_check(const formula*siki)
{
	if(siki==nullptr)
	{
		return make_pair(set<string>(),true);
	}
	pair<set<string>,bool>l=term_check(siki->l);
	pair<set<string>,bool>r=term_check(siki->r);
	if(!l.second||!r.second)return make_pair(set<string>(),false);
	set<string>now=l.first;
	for(string s:r.first)
	{
		if(!now.insert(s).second)return make_pair(set<string>(),false);
	}
	if(siki->node==FORALL||siki->node==EXISTS)
	{
		string name=siki->l->node;
		if(!now.insert(name).second)return make_pair(set<string>(),false);
	}
	return make_pair(now,true);
}
const formula*meidai(const string&s)
{
	int id=0;
	const formula*ret=meidai(id,s);
	if(id!=s.size()||ret==ERROR)
	{
		cout<<"[ERROR LOG] failed to parsing "<<s<<endl;
		return ERROR;
	}
	if(!term_check(ret).second)
	{
		cout<<"[ERROR LOG] invalid term name "<<s<<endl;
		return ERROR;
	}
	return ret;
}

const formula*rewrite(const formula*siki,const string&from,const string&to)
{
	if(siki==nullptr)return nullptr;
	string node=siki->node;
	if(isterm(siki)&&node==from)node=to;
	const formula*l=rewrite(siki->l,from,to);
	const formula*r=rewrite(siki->r,from,to);
	const formula*ret=new formula(l,r,node);
	return ret;
}

struct rule{
	int op;
	int lhs,rhs;
	int x,y,z;
	void disp()const
	{
		switch(op)
		{
			case A:
				cout<<"A";
				break;
			case MPP:
				cout<<"("<<lhs+1<<")("<<rhs+1<<")MPP";
				break;
			case MTT:
				cout<<"("<<lhs+1<<")("<<rhs+1<<")MTT";
				break;
			case DN:
				cout<<"("<<lhs+1<<")DN";
				break;
			case CP:
				cout<<"("<<lhs+1<<")("<<rhs+1<<")CP";
				break;
			case AI:
				cout<<"("<<lhs+1<<")("<<rhs+1<<")AI";
				break;
			case AE:
				cout<<"("<<lhs+1<<")AE";
				break;
			case OI:
				cout<<"("<<lhs+1<<")OI";
				break;
			case OE:
				cout<<"("<<lhs+1<<")("<<rhs+1<<")("<<x+1<<")("<<y+1<<")("<<z+1<<")OE";
				break;
			case RAA:
				cout<<"("<<lhs+1<<")("<<rhs+1<<")RAA";
				break;
			case UI:
				cout<<"("<<lhs+1<<")UI";
				break;
			case UE:
				cout<<"("<<lhs+1<<")UE";
				break;
			case EI:
				cout<<"("<<lhs+1<<")EI";
				break;
			case EE:
				cout<<"("<<x+1<<")("<<y+1<<")("<<z+1<<")EE";
				break;
			case flip:
				cout<<"("<<lhs+1<<")flip";
				break;
			default:
				cout<<"undefined";
				break;
		}
	}
};
struct gyou{
	set<int>izon;
	int number;
	const formula *siki;
	rule kisoku;
	void disp()const
	{
		for(set<int>::iterator it=izon.begin();it!=izon.end();)
		{
			cout<<*it+1;
			it++;
			if(it!=izon.end())cout<<",";
		}
		cout<<" | ("<<number+1<<") | "<<siki->to_s()<<" | ";
		kisoku.disp();
		cout<<endl;
	}
};

set<int>merge(set<int>L,const set<int>R)
{
	for(int r:R)L.insert(r);
	return L;
}
vector<gyou>syoumei;
bool disp(int id)
{
	if(!(0<=id&&id<syoumei.size()))return false;
	syoumei[id].disp();
	return true;
}
void disp_all()
{
	for(int id=0;id<syoumei.size();id++)disp(id);
}

vector<pair<pair<int,vector<int> >,pair<const formula*,const formula*> > >context;
bool add(const set<int>s,const formula*siki,const rule kisoku)
{
	int num=syoumei.size();
	syoumei.push_back({s,num,siki,kisoku});
	cout<<"add : ";
	disp(num);
	if(!context.empty())
	{
		switch(context.back().first.first)
		{
			case CP:
				if(context.back().second.second->eq(siki))
				{
					int lhs=context.back().first.second[0];
					set<int>now=s;
					if(now.find(lhs)!=now.end())
					{
						now.erase(lhs);
						int rhs=num;
						const formula*context_siki=new formula(context.back().second.first,context.back().second.second,COND);
						context.pop_back();
						add(now,context_siki,{CP,lhs,rhs});
					}
				}
				break;
			case OE:
				if(context.back().second.second->eq(siki))
				{
					int lhs=context.back().first.second[1];
					set<int>now=s;
					cout<<lhs<<endl;
					for(int dep:now)cout<<dep<<", ";
					cout<<endl;
					if(now.find(lhs)!=now.end())
					{
						now.erase(lhs);
						context.back().first.first=~OE;
						context.back().first.second.push_back(num);
						int Anum=syoumei.size();
						context.back().first.second.push_back(Anum);
						for(int depend:now)context.back().first.second.push_back(depend);
						cout<<"Second, let's proof ("<<context.back().second.first->r->to_s()<<COND<<siki->to_s()<<")"<<endl;
						add({Anum},context.back().second.first->r,{A});
					}
				}
				break;
			case ~OE:
				if(context.back().second.second->eq(siki))
				{
					int lhs=context.back().first.second[3];
					set<int>now=s;
					if(now.find(lhs)!=now.end())
					{
						now.erase(lhs);
						vector<int>idx=context.back().first.second;
						while(idx.size()>4)
						{
							now.insert(idx.back());
							idx.pop_back();
						}
						context.pop_back();
						for(int depend:syoumei[idx[0]].izon)now.insert(depend);
						add(now,siki,{OE,idx[0],idx[1],idx[2],idx[3],num});
					}
				}
				break;
			case RAA:
				if(siki->node==AND
					&&siki->r->node==NOT
					&&siki->l->eq(siki->r->r))
				{
					int lhs=context.back().first.second[0];
					set<int>now=s;
					if(now.find(lhs)!=now.end())
					{
						now.erase(lhs);
						int rhs=num;
						const formula*context_siki=new formula(nullptr,context.back().second.first,NOT);
						context.pop_back();
						add(now,context_siki,{RAA,lhs,rhs});
					}
				}
				break;
			case EE:
				if(context.back().second.first->eq(siki))
				{
					int lhs=context.back().first.second[0];
					int rhs=context.back().first.second[1];
					set<int>now=s;
					if(now.find(rhs)!=now.end())
					{
						now.erase(rhs);
						context.pop_back();
						for(int depend:syoumei[lhs].izon)now.insert(depend);
						add(now,siki,{EE,-1,-1,lhs,rhs,num});
					}
				}
				break;
		}
	}
	for(int id=0;id<syoumei.size();id++)
	{
		if(syoumei[id].siki->node!=COND)continue;
		{//MPP
			if(syoumei[id].siki->l->eq(siki))
			{
				set<int>now=merge(syoumei[id].izon,s);
				add(now,syoumei[id].siki->r,{MPP,syoumei[id].number,num});
			}
		}
		if(siki->node==NOT)
		{//MTT
			if(syoumei[id].siki->r->eq(siki->r))
			{
				set<int>now=merge(syoumei[id].izon,s);
				const formula*MTT_siki=new formula(nullptr,syoumei[id].siki->l,NOT);
				add(now,MTT_siki,{MTT,syoumei[id].number,num});
			}
		}
	}
	return true;
}

bool add_A(const formula*siki)
{
	int num=syoumei.size();
	return add({num},siki,{A});
}
bool add_A(const string&s)
{
	const formula*siki=meidai(s);
	if(siki==ERROR)return false;
	return add_A(siki);
}

bool add_MPP(int lhs,int rhs)
{
	if(syoumei[lhs].siki->node!=COND)return false;
	if(!syoumei[lhs].siki->l->eq(syoumei[rhs].siki))return false;
	set<int>now=merge(syoumei[lhs].izon,syoumei[rhs].izon);
	return add(now,syoumei[lhs].siki->r,{MPP,lhs,rhs});
}

bool add_MTT(int lhs,int rhs)
{
	if(syoumei[lhs].siki->node!=COND)return false;
	if(syoumei[rhs].siki->node!=NOT)return false;
	if(!syoumei[lhs].siki->r->eq(syoumei[rhs].siki->r))return false;
	set<int>now=merge(syoumei[lhs].izon,syoumei[rhs].izon);
	const formula*siki=new formula(nullptr,syoumei[lhs].siki->l,NOT);
	return add(now,siki,{MTT,lhs,rhs});
}

bool add_DN_ins(int id)
{
	const formula*one=new formula(nullptr,syoumei[id].siki,NOT);
	const formula*two=new formula(nullptr,one,NOT);
	return add(syoumei[id].izon,two,{DN,id});
}

bool add_DN_del(int id)
{
	if(syoumei[id].siki->node!=NOT)return false;
	const formula*one=syoumei[id].siki->r;
	if(one->node!=NOT)return false;
	const formula*two=one->r;
	return add(syoumei[id].izon,two,{DN,id});
}

bool add_AI(int lhs,int rhs)
{
	const formula*siki=new formula(syoumei[lhs].siki,syoumei[rhs].siki,AND);
	set<int>now=merge(syoumei[lhs].izon,syoumei[rhs].izon);
	return add(now,siki,{AI,lhs,rhs});
}

bool add_AE(int id,bool leave_left)
{
	if(syoumei[id].siki->node!=AND)return false;
	const formula*now=leave_left?syoumei[id].siki->l:syoumei[id].siki->r;
	return add(syoumei[id].izon,now,{AE,id});
}

bool add_OI(int id,const formula*siki)
{
	const formula*now=new formula(syoumei[id].siki,siki,OR);
	return add(syoumei[id].izon,now,{OI,id});
}
bool add_OI(int id,const string&s)
{
	const formula*siki=meidai(s);
	if(siki==ERROR)return false;
	return add_OI(id,siki);
}
bool add_OI(const formula*siki,int id)
{
	const formula*now=new formula(siki,syoumei[id].siki,OR);
	return add(syoumei[id].izon,now,{OI,id,-1});
}
bool add_OI(const string&s,int id)
{
	const formula*siki=meidai(s);
	if(siki==ERROR)return false;
	return add_OI(siki,id);
}

bool add_UI(int id,const string&from,const string&to)
{
	const formula*r=rewrite(syoumei[id].siki,from,to);
	const formula*l=new formula(to);
	const formula*now=new formula(l,r,FORALL);
	return add(syoumei[id].izon,now,{UI,id});
}
bool add_UE(int id,const string&to)
{
	if(syoumei[id].siki->node!=FORALL)return false;
	const formula*now=rewrite(syoumei[id].siki->r,syoumei[id].siki->l->node,to);
	return add(syoumei[id].izon,now,{UE,id});
}
bool add_EI(int id,const string&from,const string&to)
{
	const formula*l=new formula(to);
	const formula*r=rewrite(syoumei[id].siki,from,to);
	const formula*now=new formula(l,r,EXISTS);
	return add(syoumei[id].izon,now,{EI,id});
}

bool add_flip_FORALL(int id,const formula*siki)
{
	if(siki->node!=FORALL)return false;
	const formula*r=new formula(nullptr,siki->r,NOT);
	const formula*now=new formula(siki->l,r,EXISTS);
	return add(syoumei[id].izon,now,{flip,id});
}
bool add_flip_EXISTS(int id,const formula*siki)
{
	if(siki->node!=EXISTS)return false;
	const formula*r=new formula(nullptr,siki->r,NOT);
	const formula*now=new formula(siki->l,r,FORALL);
	return add(syoumei[id].izon,now,{flip,id});
}
bool add_flip(int id,const formula*siki)
{
	if(siki->node==FORALL)
	{
		return add_flip_FORALL(id,siki);
	}
	else if(siki->node==EXISTS)
	{
		return add_flip_EXISTS(id,siki);
	}
	return false;
}

void prompt()
{
	cout<<endl;
	for(int id=0;id<context.size();id++)
	{
		cout<<"[";
		switch(context[id].first.first)
		{
			case CP:
				cout<<"CP : "<<context[id].second.first->to_s()<<COND<<context[id].second.second->to_s();
				break;
			case OE:
				cout<<"OE : "<<context[id].second.first->l->to_s()<<COND<<context[id].second.second->to_s();
				break;
			case ~OE:
				cout<<"OE : "<<context[id].second.first->r->to_s()<<COND<<context[id].second.second->to_s();
				break;
			case RAA:
				cout<<"RAA : "<<context[id].second.first->to_s();
				break;
			case EE:
				cout<<"EE : "<<context[id].second.first->to_s();
				break;
		}
		cout<<"]";
	}
	cout<<"$ "<<flush;
}
string input()
{
	cout<<flush;
	string s;
	getline(cin,s);
	return s;
}
int input_num()
{
	return stoi(input());
}

const string A_text="A : Assumption";
const string MPP_text="MPP : Modus Ponendo Ponens, ((A"+COND+"B)"+AND+"A)"+COND+"B";
const string MTT_text="MTT : Modus Tolendo Tolens, ((A"+COND+"B)"+AND+NOT+"B)"+COND+NOT+"A";
const string DN_text="DN : Double Negation, A"+COND+NOT+NOT+"A or "+NOT+NOT+"A"+COND+"A";
const string CP_text="CP : Conditional Proof, (A|-B)"+COND+"(A"+COND+"B)";
const string AI_text="AI : AND-Introduction, (A,B)"+COND+"(A"+AND+"B)";
const string AE_text="AE : AND-Elimination, (A"+AND+"B)"+COND+"A or (A"+AND+"B)"+COND+"B";
const string OI_text="OI : OR-Introduction, A"+COND+"(A"+OR+"B) or B"+COND+"(A"+OR+"B)";
const string OE_text="OE : OR-Elimination, ((A"+OR+"B), A|-C, B|-C)"+COND+"C";
const string RAA_text="RAA : Reductio ad Absurdum, (A|-(B"+AND+NOT+"B))"+COND+NOT+"A";
const string UI_text="UI : Universal-Instroduction, Fa"+COND+"("+FORALL+"x.Fx)";
const string UE_text="UE : Universal-Elimination, ("+FORALL+"x.Fx)"+COND+"Fa";
const string EI_text="EI : Existential-Instroduction, Fa"+COND+"("+EXISTS+"x.Fx)";
const string EE_text="EE : Existential-Elimination, (("+EXISTS+"x.Fx), Fa|-A)"+COND+"A";
const string flip_text="flip : flip Universal and Existential, "+NOT+"("+FORALL+"x.Fx)"+COND+"("+EXISTS+"x."+NOT+"Fx) or "+NOT+"("+EXISTS+"x.Fx)"+COND+"("+FORALL+"x."+NOT+"Fx)";
const string del_text="del : delete the last formula [danger : DO NOT consider context]";
const string QED_text="QED : terminate this program and make proof of the last formula";
void op_help()
{
	cout<<"operation list:"<<endl
		<<A_text<<endl
		<<MPP_text<<endl
		<<MTT_text<<endl
		<<DN_text<<endl
		<<CP_text<<endl
		<<AI_text<<endl
		<<AE_text<<endl
		<<OI_text<<endl
		<<OE_text<<endl
		<<RAA_text<<endl
		<<UI_text<<endl
		<<UE_text<<endl
		<<EI_text<<endl
		<<EE_text<<endl
		<<flip_text<<endl
		<<del_text<<endl
		<<QED_text<<endl
		<<"help : show this message and now status"<<endl;
}

int main()
{
	vector<const formula*>Assumption;

	cout<<"WELCOME!"<<endl;
	op_help();

	while(true)
	{
		prompt();
		string op=input();
		bool success=false;
		if(op=="help")
		{
			op_help();
			cout<<endl<<"now status:"<<endl;
			disp_all();
			cout<<endl;
			success=true;
		}
		else if(op=="flip")
		{
			cout<<flip_text<<endl;
			cout<<"Enter target line number $ ";
			int id=input_num()-1;
			if(disp(id)&&syoumei[id].siki->node==NOT)
			{
				success=add_flip(id,syoumei[id].siki->r);
			}
		}
		else if(op=="del")
		{
			cout<<del_text<<endl;
			if(!syoumei.empty())
			{
				disp(syoumei.size()-1);
				syoumei.pop_back();
				success=true;
			}
		}
		else if(op=="QED")
		{
			cout<<QED_text<<endl;
			break;
		}
		else if(op=="A")
		{
			cout<<A_text<<endl;
			cout<<"Enter any formula $ ";
			success=add_A(input());
			if(success)Assumption.push_back(syoumei.back().siki);
		}
		else if(op=="MPP")
		{
			cout<<MPP_text<<endl;
			cout<<"Enter (A"+COND+"B) line number $ ";
			int lhs=input_num()-1;
			if(disp(lhs)&&syoumei[lhs].siki->node==COND)
			{
				cout<<"Enter A line number $ ";
				int rhs=input_num()-1;
				if(disp(rhs))
				{
					success=add_MPP(lhs,rhs);
				}
			}
		}
		else if(op=="MTT")
		{
			cout<<MTT_text<<endl;
			cout<<"Enter (A"+COND+"B) line number $ ";
			int lhs=input_num()-1;
			if(disp(lhs)&&syoumei[lhs].siki->node==COND)
			{
				cout<<"Enter "+NOT+"B line number $ ";
				int rhs=input_num()-1;
				if(disp(rhs))
				{
					success=add_MTT(lhs,rhs);
				}
			}
		}
		else if(op=="CP")
		{
			cout<<CP_text<<endl;
			cout<<"Enter formula A $ ";
			string As=input();
			const formula*Af=meidai(As);
			if(Af!=ERROR)
			{
				cout<<"Enter formula B $ ";
				string Bs=input();
				const formula*Bf=meidai(Bs);
				if(Bf!=ERROR)
				{
					cout<<"Let's proof "<<Bf->to_s()<<endl;
					context.push_back(make_pair(make_pair(CP,vector<int>{(int)syoumei.size()}),make_pair(Af,Bf)));
					success=add_A(Af);
				}
			}
		}
		else if(op=="DN")
		{
			cout<<DN_text<<endl;
			cout<<"Enter A line number $ ";
			int id=input_num()-1;
			if(disp(id))
			{
				cout<<"1. A"+COND+NOT+NOT+"A; 2. "+NOT+NOT+"A"+COND+"A; which ? [1/2] $ ";
				int t=input_num();
				if(t==1)success=add_DN_ins(id);
				else if(t==2)success=add_DN_del(id);
			}
		}
		else if(op=="AI")
		{
			cout<<AI_text<<endl;
			cout<<"Enter A line number $ ";
			int lhs=input_num()-1;
			if(disp(lhs))
			{
				cout<<"Enter B line number $ ";
				int rhs=input_num()-1;
				if(disp(rhs))
				{
					success=add_AI(lhs,rhs);
				}
			}
		}
		else if(op=="AE")
		{
			cout<<AE_text<<endl;
			cout<<"Enter (A"+AND+"B) line number $ ";
			int id=input_num()-1;
			if(disp(id)&&syoumei[id].siki->node==AND)
			{
				cout<<"leave left or right ? [l/r] $ ";
				string lr=input();
				if(lr=="l"||lr=="r")
				{
					success=add_AE(id,lr=="l");
				}
			}
		}
		else if(op=="OI")
		{
			cout<<OI_text<<endl;
			cout<<"Enter line number $ ";
			int id=input_num()-1;
			if(disp(id))
			{
				cout<<"insert left or right ? [l/r] $ ";
				string lr=input();
				if(lr=="l"||lr=="r")
				{
					cout<<"Enter any formula $ ";
					string s=input();
					success=lr=="l"?add_OI(s,id):add_OI(id,s);
				}
			}
		}
		else if(op=="OE")
		{
			cout<<OE_text<<endl;
			cout<<"Enter (A"+OR+"B) line number $ ";
			int id=input_num()-1;
			if(disp(id)&&syoumei[id].siki->node==OR)
			{
				cout<<"Entery formula C $ ";
				string Cs=input();
				const formula*Cf=meidai(Cs);
				if(Cf!=ERROR)
				{
					cout<<"Let's proof ("<<syoumei[id].siki->to_s()<<COND<<Cf->to_s()<<")"<<endl;
					cout<<"First, let's proof ("<<syoumei[id].siki->l->to_s()<<COND<<Cf->to_s()<<")"<<endl;
					context.push_back(make_pair(make_pair(OE,vector<int>{syoumei[id].number,(int)syoumei.size()}),make_pair(syoumei[id].siki,Cf)));
					success=add_A(syoumei[id].siki->l);
				}
			}
		}
		else if(op=="RAA")
		{
			cout<<RAA_text<<endl;
			cout<<"Enter formula A $ ";
			string As=input();
			const formula*Af=meidai(As);
			if(Af!=ERROR)
			{
				cout<<"Let's proof any contradiction B"+AND+NOT+"B"<<endl;
				context.push_back(make_pair(make_pair(RAA,vector<int>{(int)syoumei.size()}),make_pair(Af,ERROR)));
				success=add_A(Af);
			}
		}
		else if(op=="UI")
		{
			cout<<UI_text<<endl;
			cout<<"Enter Fa line number $ ";
			int id=input_num()-1;
			cout<<"Enter term a's name $ ";
			string from=input();
			if(disp(id)&&has_term(syoumei[id].siki,from))
			{
				cout<<"Enter new term name $ ";
				string to=input();
				if(!has_term(syoumei[id].siki,to))
				{
					success=add_UI(id,from,to);
				}
			}
		}
		else if(op=="UE")
		{
			cout<<UE_text<<endl;
			cout<<"Enter ("+FORALL+"x.Fx) line number $ ";
			int id=input_num()-1;
			if(disp(id)&&syoumei[id].siki->node==FORALL)
			{
				cout<<"Enter new term name $ ";
				string to=input();
				if(!has_term(syoumei[id].siki,to))
				{
					success=add_UE(id,to);
				}
			}
		}
		else if(op=="EI")
		{
			cout<<EI_text<<endl;
			cout<<"Enter Fa line number $ ";
			int id=input_num()-1;
			cout<<"Enter term a's name $ ";
			string from=input();
			if(disp(id)&&has_term(syoumei[id].siki,from))
			{
				cout<<"Enter new term name $ ";
				string to=input();
				if(!has_term(syoumei[id].siki,to))
				{
					success=add_EI(id,from,to);
				}
			}
		}
		else if(op=="EE")
		{
			cout<<EE_text<<endl;
			cout<<"Enter ("+EXISTS+"x.Fx) line number $ ";
			int id=input_num()-1;
			if(disp(id)&&syoumei[id].siki->node==EXISTS)
			{
				cout<<"Enter formula A $ ";
				string As=input();
				const formula*Af=meidai(As);
				if(Af!=ERROR)
				{
					cout<<"Enter new term name $ ";
					string to=input();
					if(!has_term(syoumei[id].siki,to)&&!has_term(Af,to))
					{
						const formula*now=rewrite(syoumei[id].siki->r,syoumei[id].siki->l->node,to);
						cout<<"Let's proof ("<<now->to_s()<<COND<<Af->to_s()<<")"<<endl;
						context.push_back(make_pair(make_pair(EE,vector<int>{syoumei[id].number,(int)syoumei.size()}),make_pair(Af,ERROR)));
						success=add_A(now);
					}
				}
			}
		}

		if(success)
		{
			cout<<"Success"<<endl;
		}
		else
		{
			cout<<"Invalid"<<endl;
		}
	}

	cout<<endl;
	cout<<"now status:"<<endl;
	disp_all();
	cout<<endl;
	bool ok=false;
	if(Assumption.size()==syoumei.back().izon.size())
	{
		ok=true;
		int id=0;
		for(int jd:syoumei.back().izon)if(!syoumei[jd].siki->eq(Assumption[id++]))ok=false;
	}
	if(ok)
	{
		cout<<"Proof of ";
		for(int i=0;i<Assumption.size();i++)
		{
			cout<<Assumption[i]->to_s();
			if(i+1<Assumption.size())cout<<", ";
		}
		cout<<" |- ";
		cout<<syoumei.back().siki->to_s()<<endl<<endl;
		disp_all();
	}
	else
	{
		cout<<"Faild"<<endl;
	}
}
