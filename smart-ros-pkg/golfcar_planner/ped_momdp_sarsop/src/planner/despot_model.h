#ifndef PEDESTRIAN_H
#define PEDESTRIAN_H

#include <cmath>
#include <utility>
#include <algorithm>
#include "globals.h"
#include "model.h"
#include "coord.h"
#include "lower_bound/lower_bound_policy_mode.h"
#include "lower_bound/lower_bound_policy_random.h"
#include "upper_bound/upper_bound_stochastic.h"
#include "util_uniform.h"
#include "string.h"
#include "SFM.h"
#include "pedestrian_state.h"

int Y_SIZE;
int X_SIZE;
int N_GOAL;

struct PathNode
{
	vector<PathNode> children;	
	int x,y;
};
template<>
class Model<PomdpState> : public IUpperBound<PomdpState>
{
	public:
		Model(const RandomStreams& streams, string filename);
		void SetStartState(PomdpState& state);
		PomdpState GetStartState() const;
		//vector<PomdpState> GetStartStates(int num_ped) const;
		uint64_t Observe(const PomdpState& state) const; // Observation for non-terminal state
		vector<int> ObserveVector(const PomdpState& state)   const;
		void RobStep(int &robY,int &rob_vel, int action, UtilUniform &unif) const;
		void PedStep(PomdpState& state, UtilUniform &unif) const;
		void StepMultiple(vector<PomdpState>& states, double rNum, int action, vector<double>& rewards, vector<uint64_t>& obss) const;
		void Step(PomdpState& state, double rNum, int action, double& reward, uint64_t& obs) const;
		/* TODO: bug due to not passing by reference for state - UpperBound computation*/
		void Step(PomdpState& state, double rNum, int action, double& reward) const {
			uint64_t obs;
			return Step(state, rNum, action, reward, obs);
		}
		double ObsProb(uint64_t z, const PomdpState s, int action) const;
		double TransProbJoint(const PomdpState& s_old, const PomdpState& s_new, int action) const; 
		double FringeUpperBound(const PomdpState& s) const;

		double UpperBound(const vector<Particle<PomdpState>*>& particles,
				const Model<PomdpState>& model,
				int stream_position,
				History& history) const {
			double ub = 0;
			for(auto particle : particles)
				ub += FringeUpperBound(particle->state);
			return ub / particles.size();
		}

		double FringeLowerBound(const vector<Particle<PomdpState>*>& particles) const;
		vector<pair<PomdpState, double>> InitialBelief() const;

		PomdpState RandomState(unsigned& seed, PomdpState obs_state) const {
			double prob;
			for(int i=0;i<obs_state.num;i++)
			{
				obs_state.PedPoses[i].second=rand_r(&seed)%ModelParams::NGOAL;
			}
			return obs_state;
		}

		void InitModel();
		void EnumerateBelief(vector<pair<PomdpState, double>> &belief,PomdpState state,int depth) const
		{
			if(depth==0) {
				belief.push_back(pair<PomdpState,double>(state, 1.0/pow(ModelParams::NGOAL,state.num)));
				return;
			}

			for(int i=0;i<ModelParams::NGOAL;i++)
			{
				int x=state.PedPoses[depth-1].first.X;
				int y=state.PedPoses[depth-1].first.Y;
				if(fabs(sfm->local_goals[i][0]-x)<ModelParams::GOAL_DIST&&fabs(sfm->local_goals[i][1]-y)<ModelParams::GOAL_DIST)
				{
					state.PedPoses[depth-1].second=i;
					EnumerateBelief(belief,state,depth-1);
				}
			}
		}
		
		vector<vector<double> > GetBeliefVector(const vector<Particle<PomdpState>*> particles) const
		{
			double goal_count[10][10]={0};


			PomdpState state_0=particles[0]->state;
			for(int i=0;i<particles.size();i++)
			{
				PomdpState state=particles[i]->state;
				for(int j=0;j<state.num;j++)
				{
						goal_count[j][state.PedPoses[j].second]+=particles[i]->wt;
				}
			}
			
			vector<vector<double> > belief_vec;
			for(int j=0;j<state_0.num;j++)
			{
				vector<double> belief;
				for(int i=0;i<ModelParams::NGOAL;i++)
				{	
					//belief.push_back((goal_count[j][i]+0.0)/particles.size());
					belief.push_back((goal_count[j][i]+0.0));
				}
				belief_vec.push_back(belief);
			}
			return belief_vec;
		}



		int DefaultPolicy(const vector<Particle<PomdpState>*> particles) const 
		{
			PomdpState state=particles[0]->state;
			int robY=state.RobPos.Y;		
			int rx=rob_map[robY].first;
			int ry=rob_map[robY].second;
			double rate=ModelParams::map_rln/ModelParams::rln;
			
			int rangeX=(ModelParams::map_rln/ModelParams::rln)*1;
			int rangeY=(ModelParams::map_rln/ModelParams::rln)*3;
			for(int i=0;i<state.num;i++)
			{
				int px=state.PedPoses[i].first.X;
				int py=state.PedPoses[i].first.Y;
				int crash_point=sfm->crash_model[px][py];
				int crashx=rob_map[crash_point].first;
				int crashy=rob_map[crash_point].second;
				if(abs(px-crashx)<=rangeX&&crashy-ry>=-rate&&crashy-ry<=rangeY)
				{
					return 2;
				}
				//if(abs(px-crashx)<=rangeX*2+2&&crashy-ry>=-4&&crashy-ry<=rangeY*2+2)
				if(abs(px-crashx)<=rangeX*2&&crashy-ry>=-rate&&crashy-ry<=rangeY*2)
				{
					//if(state.Vel==0) return 1;
					//else   return 0;

					double unit=ModelParams::VEL_MAX/(ModelParams::VEL_N-1);
					if(state.Vel>1.0/unit) return 2;
					else if(state.Vel<0.5/unit) return 1;
					else return 0;
				}
			}
			return 1;
		}
		void Statistics(const vector<Particle<PomdpState>*> particles) const {
			
			double goal_count[10][10]={0};
			cout<<"Current Belief "<<endl;
			cout<<"particles num "<<particles.size()<<endl;
			if(particles.size()==0) return;
			//if(particles.size()==0) return ;
			//cout<<"first particle "<<endl;
			PrintState(particles[0]->state);
			PomdpState state_0=particles[0]->state;
			for(int i=0;i<particles.size();i++)
			{
				PomdpState state=particles[i]->state;
				for(int j=0;j<state.num;j++)
				{
						goal_count[j][state.PedPoses[j].second]+=particles[i]->wt;
				}
			}
			
			for(int j=0;j<state_0.num;j++)
			{
				cout<<"Ped "<<j<<" Belief is ";
				for(int i=0;i<N_GOAL;i++)
				{
					cout<<(goal_count[j][i]+0.0)<<" ";
				}
				cout<<endl;
			}
		}

		void ModifyObsStates(const vector<Particle<PomdpState>*> &particles,PomdpState&new_state,unsigned & seed) const
		{
			int num_ped = new_state.num;

			// Copy pedestrians and goals of existing pedestrians
			for (int i=0; i<particles.size(); i++) {
				PomdpState state = particles[i]->state;
				for (int j=0; j<num_ped; j++) {
					new_state.PedPoses[j].second = -1;
					for(int k=0;k<state.num;k++)
					{
						if(state.PedPoses[k].third==new_state.PedPoses[j].third)  {
							new_state.PedPoses[j].second=state.PedPoses[k].second;
							break;
						}
					}
				}
				particles[i]->state = new_state;
			}

			// Assign goal to new pedestrians
			for(int j=0;j<num_ped;j++)   {
				vector<int> perm;
				for (int i=0; i<particles.size(); i++)
					perm.push_back(i);
				random_shuffle(perm.begin(), perm.end());
				double cur_wgt = 0, cur_goal = 0, unit = 1.0 / ModelParams::NGOAL;

				for(int i : perm) {
					PomdpState state=particles[i]->state;
					if(state.PedPoses[j].second==-1)  
						particles[i]->state.PedPoses[j].second = cur_goal;

					cur_wgt += particles[i]->wt;
					if (cur_wgt >= unit) {
						cur_goal ++;
						unit += 1.0 /ModelParams::NGOAL;
					}
				}
			}
		}

		void PrintState(const PomdpState& state, ostream& out = cout) const;
		void PrintObs(uint64_t obs, ostream& out = cout) const { out << obs; }

		int NumStates() const { 
			cout<<"print NumStates"<<endl;
			return pow((X_SIZE * Y_SIZE*N_GOAL),ModelParams::N_PED_IN) * ModelParams::RMMax* ModelParams::VEL_N +1;  //plus one dummy state	
		}
		int NumActions() const { return 3; }
		bool IsTerminal(PomdpState s) const {
			return s.Vel == -1;
		}

		uint64_t MaxObs() const {
			double pedObsRate=ModelParams::rln/ModelParams::ped_rln;
			return pow(int(X_SIZE*pedObsRate)*int(Y_SIZE*pedObsRate),ModelParams::N_PED_IN)*(ModelParams::RMMax)*ModelParams::VEL_N+2;
		}
		uint64_t TerminalObs() const {
			return MaxObs()-1;
		}

		int LowerBoundAction(const PomdpState& s) {
			return ACT_CUR;
		}

		Particle<PomdpState>* Copy(const Particle<PomdpState>* particle) const {
			Particle<PomdpState>* new_particle = Allocate();
			*new_particle = *particle;
			return new_particle;
		}

		Particle<PomdpState>* Allocate() const {
			return memory_pool_.Allocate();
		}

		void Free(Particle<PomdpState>* particle) const {
			memory_pool_.Free(particle);
		}

		shared_ptr<vector<int>> GeneratePreferred(const PomdpState& state,
				const History& history) const {
			return shared_ptr<vector<int>>(new vector<int>());
		}

		shared_ptr<vector<int>> GenerateLegal(const PomdpState& state,
				const History& history) const {
			static shared_ptr<vector<int>> actions = nullptr;
			if (!actions) {
				int num_actions = NumActions();
				actions = shared_ptr<vector<int>>(new vector<int>(num_actions));
				for (int a = 0; a < num_actions; a++)
					(*actions)[a] = a;
			}
			return actions;
		}


	//double model[ModelParams::XSIZE][ModelParams::YSIZE][ModelParams::NGOAL][ModelParams::R][9];  
	std::vector<std::pair<int,int> > rob_map;
	//int rob_map[ModelParams::XSIZE][ModelParams::YSIZE];   //modify rob map as a boolean grid indicates whether each cell is occupied by the path
	SFM*sfm;
	double control_freq;

	protected:
		double OBSTACLE_PROB;

		enum
		{
			ACT_CUR,
			ACT_ACC,
			ACT_DEC
		};
	private:
		int** map;
		PomdpState startState;
		mutable MemoryPool<Particle<PomdpState>> memory_pool_;
		int configSeed;
		mutable UtilUniform unif_;

		double robotNoisyMove[3][3]; /*vel,move*/
		double robotMoveProbs[3][3]; 
		double robotVelUpdate[3][3][3]; /*action,vel,new vel*/ 
		double robotUpdateProb[3][3][3];

		int lookup(const double probs[], double prob) const {
			int pos = 0;
			double sum = probs[0];
			while(sum < prob) {
				pos ++;
				sum += probs[pos];
			}
			return pos;
		}
		double gaussian(double dist)  const{
			if(dist<0.1) dist=0.1;
			return 1/dist;
		}
};

const int CRASH_PENALTY =-1000;
const int GOAL_REWARD = 500;

Model<PomdpState>::Model(const RandomStreams& streams, string filename) : IUpperBound<PomdpState>(streams)
{
	//ifstream fin(filename, ios::in);
	X_SIZE = ModelParams::XSIZE;
	N_GOAL=ModelParams::NGOAL;

	//fin >> Y_SIZE;
	Y_SIZE=ModelParams::YSIZE;
	//fin.close();

	configSeed = Globals::config.root_seed ^ (Globals::config.n_particles + 3); // TODO: set config seed in main.cpp
	unif_ = UtilUniform((1.0 * configSeed) / RAND_MAX);
	OBSTACLE_PROB = 0.0;

	//InitModel();

	double noisyMove[3][3] /*vel, move*/ = {{0, 1, 0},
		{0, 1, 2},
		//{1, 2, 3}};
		{0, 1, 2}};
	memcpy(robotNoisyMove, noisyMove, sizeof(noisyMove));
	if(ModelParams::goodrob==0)
	{
	   double moveProbs[3][3] = {{0.9, 0.1, 0.0},
	   {0.1, 0.8, 0.1},
	//	{0.1, 0.8, 0.1}};
		{0.1, 0.1, 0.8}};
		memcpy(robotMoveProbs, moveProbs, sizeof(moveProbs));
	}
	else
	{

		double moveProbs[3][3] = {{1.0, 0.0, 0.0},
			{0.1, 0.8, 0.1},
			//	{0.1, 0.8, 0.1}};
			{0.1, 0.1, 0.8}};
		memcpy(robotMoveProbs, moveProbs, sizeof(moveProbs));
	}


	double velUpdate[3][3][3] /*action,vel,updated*/ = {
		{{0, 1, 0}, {0, 1, 2}, {0, 1, 2}},
		{{0, 1, 2}, {0, 1, 2}, {0, 1, 2}},
		{{0, 1, 0}, {0, 1, 0}, {0, 1, 2}}};
	memcpy(robotVelUpdate, velUpdate, sizeof(velUpdate));

	if(ModelParams::goodrob==0)
	{
		double updateProb[3][3][3] /*action,vel,updated*/ = {
			{{0.9, 0.1, 0.0}, {0.2, 0.7, 0.1}, {0.1, 0.1, 0.8}},
			{{0.2, 0.7, 0.1}, {0.1, 0.1, 0.8}, {0.1, 0.1, 0.8}},
			{{0.9, 0.1, 0.0}, {0.9, 0.1, 0.0}, {0.2, 0.7, 0.1}}};
		memcpy(robotUpdateProb, updateProb, sizeof(updateProb));
	}
	else
	{
		double updateProb[3][3][3] /*action,vel,updated*/ = {
		{{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}},
		{{0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0, 1.0}},
		{{1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.5, 0.5, 0.0}}};
		memcpy(robotUpdateProb, updateProb, sizeof(updateProb));
	}

	double prob = unif_.next(),
	sum = 0;

	vector<pair<PomdpState, double>> belief;


	PomdpState init_state;
	init_state.RobPos.Y = 0;
	init_state.Vel=1;

	/*
	for(int pedX=0; pedX<X_SIZE; pedX++) {
		for(int pedY=0; pedY<Y_SIZE; pedY++) {
			for(int goal=0; goal<N_GOAL; goal++) {

				for(int i=0;i<ModelParams::N_PED_IN;i++) {


					state.PedPoses.push_back(make_pair(COORD(pedX,pedY),goal));

					if(fabs(pedX-2)+fabs(pedY-0)<=3) continue;

					belief.push_back(pair<PomdpState,double>(state, 1.0/(X_SIZE * Y_SIZE * N_GOAL)));
				}
			}
		}
	}*/

	//assign an arbitary state
	startState.RobPos.Y=0;
	startState.Vel=1;
	startState.num=ModelParams::N_PED_IN;
	
	for(int i=0;i<startState.num;i++)
    {
		bool ok=false;
		int N,NX,NY,NG;
		while(!ok)
		{
			prob=unif_.next();
			N=prob*X_SIZE*Y_SIZE;
			NX=N/Y_SIZE;
			NY=N-(NX*Y_SIZE);
			if(fabs(NX-2)+fabs(NY-0)>3) {ok=true;}
		}
		prob=unif_.next();
		NG=N_GOAL*prob;
		//startState.PedPoses.push_back(make_pair(COORD(NX,NY),NG));
		startState.PedPoses[i]=PedStruct(COORD(NX,NY),NG,i);
	}

/*	
	for(pair<PomdpState, double> state : belief) {
		sum += state.second;
		if(sum >= prob) {
			startState = state.first;
			break;
		}
	}*/

	/*
		 auto initialBelief = InitialBelief();
		 for(pair<PomdpState, double> state : initialBelief) {
		 sum += state.second;
		 if(sum >= prob) {
		 startState = state.first;
		 break;
		 }
		 }
		 */
	/*
	for(int i=0;i<ModelParams::XSIZE;i++)
		for(int j=0;j<ModelParams::YSIZE;j++)
			rob_map[i][j]=0;
			*/
}

void Model<PomdpState>::PrintState(const PomdpState& state, ostream& ostr) const
{

	cout << "Rob Pos: " << rob_map[state.RobPos.Y].first << " " <<rob_map[state.RobPos.Y].second <<endl;
	//for(int i=0;i<state.PedPoses.size();i++)
	for(int i=0;i<state.num;i++)
	{
		cout << "Ped Pos: " << state.PedPoses[i].first.X << " " <<state.PedPoses[i].first.Y <<endl;
		cout << "Goal: " << state.PedPoses[i].second << endl;
		cout << "id: " << state.PedPoses[i].third << endl;
	}
	cout << "Vel: " << state.Vel << endl;
	cout<<  "num  " << state.num << endl;
}

void Model<PomdpState>::InitModel()
{
	map = new int*[X_SIZE];
	for(int x=0; x<X_SIZE; x++) {
		map[x] = new int[Y_SIZE];
	}

	double p;
	cerr<<OBSTACLE_PROB<<endl;
	for(int i=1; i<Y_SIZE-1; i++)
	{
		p = unif_.next(); 
		map[0][i] = (p < OBSTACLE_PROB);
	}
	for(int i=1; i<Y_SIZE-1; i++)
	{
		p = unif_.next();
		map[3][i] = (p < OBSTACLE_PROB);
	}

	cerr << "Map" << endl;
	for(int y=0; y<Y_SIZE; y++) {
		for(int x=0; x<X_SIZE; x++)
			cerr << map[x][y] << " "; 
		cerr << endl;
	}
	cerr << endl;
}

vector<int> Model<PomdpState>::ObserveVector(const PomdpState& state) const {
	vector<int> obs_vec;
	uint64_t robObs=state.Vel+state.RobPos.Y*ModelParams::VEL_N;
	double pedObsRate=ModelParams::rln/ModelParams::ped_rln;
	obs_vec.push_back(robObs);
	for(int i=0;i<state.num;i++)
	{
		int this_obs=int(state.PedPoses[i].first.X*pedObsRate)*int(Y_SIZE*pedObsRate)+int(state.PedPoses[i].first.Y*pedObsRate);	
		obs_vec.push_back(this_obs);
	}
	return obs_vec;
}
uint64_t Model<PomdpState>::Observe(const PomdpState& state) const {
	hash<vector<int>> myhash;
	return myhash(ObserveVector(state));
	uint64_t obs=0;// = state.Vel*(X_SIZE*Y_SIZE*rob_map.size())+state.RobPos.Y*(X_SIZE*Y_SIZE)+state.PedPos.X*Y_SIZE+state.PedPos.Y;
	uint64_t robObs=state.Vel+state.RobPos.Y*ModelParams::VEL_N;
	uint64_t robObsMax=ModelParams::VEL_N*ModelParams::RMMax;  //max length of the rob_map
	
	double pedObsRate=ModelParams::rln/ModelParams::ped_rln;
	uint64_t pedObsMax=uint64_t(X_SIZE*pedObsRate)*uint64_t(Y_SIZE*pedObsRate);
	uint64_t pedObs=1;
	//for(int i=0;i<state.PedPoses.size();i++)
	for(int i=0;i<state.num;i++)
	{
		uint64_t this_obs=uint64_t(state.PedPoses[i].first.X*pedObsRate)*uint64_t(Y_SIZE*pedObsRate)+uint64_t(state.PedPoses[i].first.Y*pedObsRate);	
		pedObs=pedObs*pedObsMax+this_obs;
	}
	obs=pedObs*robObsMax+robObs;
	return obs;
}

void Model<PomdpState>::RobStep(int &robY,int &rob_vel, int action, UtilUniform &unif) const {


	/*
	robY += robotNoisyMove[rob_vel][lookup(robotMoveProbs[rob_vel], p)];
	if(robY >= Y_SIZE) robY = Y_SIZE - 1;
	p = unif.next();
	rob_vel = robotVelUpdate[action][rob_vel][lookup(robotUpdateProb[action][rob_vel], p)];
	*/

	//double vmax=ModelParams::VEL_MAX/control_freq * 10.0/ModelParams::rln; // grids per control
	double vmax=ModelParams::VEL_MAX/control_freq * 10.0/ModelParams::rln; // grids per control
	double delta=vmax/(ModelParams::VEL_N-1);
	double v=rob_vel*delta;
	double next_center=robY+v;
	double weight[ModelParams::RMMax+10];
	double weight_sum=0;
	int max_dist=int(vmax*1.3+1.0);
	int counter=0;
	//cout<<"start pringting weight "<<rob_vel<<" "<<next_center<<endl;
	for(int i=robY;i<rob_map.size()&&i<=robY+max_dist;i++)
	{
		weight[i-robY]=gaussian(fabs(next_center-i));
		weight_sum+=weight[i-robY];
		counter++;
		//cout<<weight[i-robY]<<" ";
	}
	//cout<<endl;
	if(counter>10)
		cout<<"rob step counter "<<counter<<endl;

	double p = unif.next()*weight_sum;
	weight_sum=0;
	int dist=-1;
	for(int i=robY;i<rob_map.size()&&i<=robY+max_dist;i++)
	{
		weight_sum+=weight[i-robY];
		if(weight_sum>p) {
			dist=i;
			break;
		}
	}
	//cout<<dist-robY<<endl;
	robY=dist;

}

void Model<PomdpState>::PedStep(PomdpState& state, UtilUniform &unif) const
{
	/*
	int &pedX=state.PedPos.X;
	int &pedY=state.PedPos.Y;
	int goal=state.Goal;
	//double p=(rand()+0.0)/RAND_MAX;
	double p=unif.next();
	double prob=0;
	bool inner=false;
	
	int tempx=pedX,tempy=pedY;
	for(int i=0;i<X_SIZE,inner==false;i++)
		for(int j=0;j<Y_SIZE;j++)
		{
			prob+=model[pedX][pedY][goal][i][j];
			if(p<prob)
			{
				//cout<<model[pedX][pedY][goal][i][j]<<endl;
				//cout<<pedX<<" "<<pedY<<endl;
				//cout<<i<<" "<<j<<endl;
				pedX=i;
				pedY=j;
				inner=true;
				break;
			}
		}*/
	if(sfm==0) {
		cerr<<"!!!!!!SFM not initialized"<<endl;
		return;
	}
	sfm->ModelTransFast(state,unif);   //make transition using SFM Model
	
}

void Model<PomdpState>::StepMultiple(vector<PomdpState>& states, double rNum, int action, vector<double>& rewards, vector<uint64_t>& obss) const {
	UtilUniform unif(rNum);
	for(int i=0; i<states.size(); i++) {
		double reward;
		uint64_t obs;
		/*
		cout << "Before step " << i << " " << &states[i] << endl;
		PrintState(states[i]);
		*/
		Step(states[i], unif.next(), action, reward, obs);
		/*
		cout << "After step" << &states[i] << endl;
		PrintState(states[i]);
		*/

		if(i > 0) {
			states[i].RobPos = states[0].RobPos;
			states[i].Vel = states[0].Vel;
			if(obs != TerminalObs())
				obs = Observe(states[i]);
		}
		rewards.push_back(reward);
		obss.push_back(obs);
	}
}

int action_vel[3]={0,1,-1};
void UpdateVel(int &vel,int action,UtilUniform &unif)
{


	double prob=unif.next();		
	if(prob<0.2) vel=vel;
	else vel=vel+action_vel[action];
	//else if(prob<0.5) vel=vel+action_vel[action];
	//else if(prob<0.85)vel=vel+action_vel[action]*2;
	//else {
//		prob=unif.next();
//		vel=int(prob*ModelParams::VEL_N);
//	}
	if(vel<0) vel=0;	
	if(vel>ModelParams::VEL_N-1) vel=ModelParams::VEL_N-1;
}

void Model<PomdpState>::Step(PomdpState& state, double rNum, int action, double& reward, uint64_t& obs) const {

	reward = 0;
	obs = TerminalObs();
	if(world.isGoal(state)) {
		reward=GOAL_REWARD;	
		state.car.vel=-1;
		return;
	}
	double collision_penalty=world.inCollision(state,action);
	if(collision_penalty<0) {
		reward+=collision_penalty;
	}
	
	int robY = state.RobPos.Y;
	int rob_vel = state.Vel;

	/*
	for(int i=0;i<state.num;i++)
	{
		int &pedX = state.PedPoses[i].first.X;
		int &pedY = state.PedPoses[i].first.Y;
		int rx=rob_map[robY].first;
		int ry=rob_map[robY].second;

		//retrieve the closest point to the pedestrian on the path according to x coord
		int crash_point=sfm->crash_model[pedX][pedY];
		int crashx=rob_map[crash_point].first;
		int crashy=rob_map[crash_point].second;

		double rate=ModelParams::map_rln/ModelParams::rln;
		int rangeX=rate/2;
		int rangeY=rate*1.5;

		if(abs(crashx-pedX)<=rangeX&&crashy-ry>=-rate&&crashy-ry<=rangeY) 
		{
			reward+=CRASH_PENALTY * (rob_vel+1);
		}
		
		rangeX=rate*1.5;
		rangeY*=2;

		double unit=ModelParams::VEL_MAX/(ModelParams::VEL_N-1);
		if(rob_vel > 1.0/unit &&abs(crashx-pedX)<=rangeX&&crashy-ry>=-rate&&crashy-ry<=rangeY)
		{
			reward+=CRASH_PENALTY/2;
		}
	}*/

	UtilUniform unif(rNum);
	double p = unif.next();
	if(action ==2)  reward+=-10;
	else  reward+=-1;
	RobStep(robY,rob_vel, action, unif);
	UpdateVel(rob_vel,action,unif);
	if(robY >= rob_map.size()-1) robY = rob_map.size() - 1;

	PedStep(state, unif);

	state.Vel=rob_vel;
	state.RobPos.Y=robY;

	obs = Observe(state);
}

double Model<PomdpState>::TransProbJoint(const PomdpState& s_old, const PomdpState& s_new, int action) const {
	double prob=1.0;
	
	prob*=sfm->ModelTransProb(s_old,s_new);
/*
	int real_vel=s_old.Vel/2;
	//robY += robotNoisyMove[rob_vel][lookup(robotMoveProbs[rob_vel], p)];
	double this_prob=0.01;
	for(int i=0;i<3;i++)
	{
		if(robotNoisyMove[real_vel][i]==s_new.RobPos.Y-s_old.RobPos.Y) 
		{
			this_prob=robotMoveProbs[real_vel][i];	
			break;
		}
	}
	prob*=this_prob;

	int v0=s_old.Vel;
	int v1=s_new.Vel;
	double vel_prob=0.01;
	if(action==1)
	{
		if(v1-v0==0) {
			vel_prob=0.15;	
			if(v0==4) vel_prob+=0.7;
		}
		else if(v1-v0==1) {
			vel_prob=0.35;
			if(v0==3) vel_prob+=0.35;
		}
		else if(v1-v0==2) {
			vel_prob=0.35;
		}
		else
		{
			//approximation
			vel_prob=0.15/5;
		}
	}
	else if(action==2)
	{
		if(v1-v0==0){
			vel_prob=0.15;
			if(v0==0) vel_prob+=0.7;
		}
		else if(v1-v0==-1) {
			vel_prob=0.35;
			if(v0==1) vel_prob+=0.35;
		}
		else if(v1-v0==-2) {
			vel_prob=0.35;
		}
		else {
			//approximatioin
			vel_prob=0.15/5;
		}
	}
	else
	{
		if(v1-v0==0) {
			vel_prob=0.85;
		}
		else {
			vel_prob=0.15/5;
		}
	}

	prob*=vel_prob;
	*/
	//if(robY >= rob_map.size()-1) robY = rob_map.size() - 1;
	//p = unif.next();

	//rob_vel = robotVelUpdate[action][rob_vel][lookup(robotUpdateProb[action][rob_vel], p)];
	
	//trans vel
    return prob;
}

PomdpState ObsToState(uint64_t obs)
{
	PomdpState state;
	uint64_t robObsMax=ModelParams::VEL_N*ModelParams::RMMax;
	uint64_t robObs=obs%robObsMax;
	state.Vel=robObs%ModelParams::VEL_N;
	state.RobPos.Y=robObs/ModelParams::VEL_N;
	uint64_t pedObs=obs/robObsMax;
	int i=0;
	uint64_t pedObsMax=X_SIZE*Y_SIZE;
	while(pedObs>1)
	{
		uint64_t this_obs;
		this_obs=pedObs%pedObsMax;
		pedObs=pedObs/pedObsMax;	
		int x,y;
		y=this_obs%Y_SIZE;
		x=this_obs/Y_SIZE;
		state.PedPoses[i].first.X=x;
		state.PedPoses[i].first.Y=y;
		i++;
	}
	state.num=i;
	for(int i=0;i<state.num/2;i++)
	{
		std::swap(state.PedPoses[i],state.PedPoses[state.num-i-1]);
	}
	return state;
}

double Model<PomdpState>::ObsProb(uint64_t obs, const PomdpState s, int action) const {
	//cout<<"obs "<<obs<<endl;
	//cout<<"Observe "<<Observe(s)<<endl;
	//PrintState(s);
	//
	return (!IsTerminal(s) && obs == Observe(s)) || (IsTerminal(s) && obs == TerminalObs());
}

double Model<PomdpState>::FringeUpperBound(const PomdpState& s) const {
	if (IsTerminal(s)) { // TODO: bug due to missing this - UpperBound table
		return 0;
	}


	double vmax=ModelParams::VEL_MAX/control_freq*ModelParams::map_rln/ModelParams::rln;
	int max_dist=int(vmax*1.3);
	int d = int((fabs(rob_map.size()/2 - s.RobPos.Y)) / (max_dist+3));
	/* TODO: There was a bug caused by not using the global discount factor - UpperBound table.*/
	return GOAL_REWARD * pow(Globals::config.discount, d);
}

double Model<PomdpState>::FringeLowerBound(const vector<Particle<PomdpState>*>& particles) const {
	//return CRASH_PENALTY*ModelParams::VEL_N;
	return CRASH_PENALTY*ModelParams::VEL_N*ModelParams::N_PED_IN / 0.05;
}

void Model<PomdpState>::SetStartState(PomdpState& state) {
	startState = state;
}

PomdpState Model<PomdpState>::GetStartState() const {
	return startState;
}

/*
vector<PomdpState> Model<PomdpState>::GetStartStates(int num_ped) const {
	
	vector<PomdpState> states;
	for(int i=0; i<num_ped; i++) {
		PomdpState state;
		state.RobPos.X = 1;
		state.RobPos.Y = 0;
		state.Vel = 1;

		state.Goal = unif_.nextInt(N_GOAL);
		state.PedPos.X = unif_.nextInt(X_SIZE);
		state.PedPos.Y = unif_.nextInt(Y_SIZE);
		states.push_back(state);
	}
	return states;
}*/



vector<pair<PomdpState, double>> Model<PomdpState>::InitialBelief() const {
	vector<pair<PomdpState, double>> belief;
	/*
	for(int i=0;i<ModelParams::N_PED;i++)
	{
		for(int goal=0; goal<N_GOAL; goal++) {
			PomdpState state = startState;
			state.PedPoses[i].second = goal;
			//state.PedPoses[i].second=startState.PedPoses[i].second;  //assume full observe

			belief.push_back(pair<PomdpState,double>(state, 1.0/(N_GOAL*ModelParams::N_PED)));
		}
	}*/
	
	PomdpState state=startState;
	EnumerateBelief(belief,startState,startState.num);
	cout<<"belief size "<<belief.size()<<endl;
	cout<<"finish initial belief"<<endl;
	return belief;
}



#endif