#include <ped_pathplan/pathplan.h>
#include <ros/console.h>
#include <vector>
#include <algorithm>
#include <set>
#include <queue>
#include <utility>

namespace ped_pathplan {

    using namespace std;


    PathPlan::PathPlan(int xs, int ys): nx(xs), ny(ys), step(2) {
        ns = xs * ys;
        costarr = new COSTTYPE[ns]; // cost array, 2d config space
        memset(costarr, 0, ns*sizeof(COSTTYPE));

        steerings.push_back(0);
        for(float f=D_YAW; f<=STEERING_LIMIT; f+=D_YAW) {
            steerings.push_back(f);
            steerings.push_back(-f);
        }
    }

    void PathPlan::setCostmap(const COSTTYPE *cmap, bool isROS, bool allow_unknown)
    {
        COSTTYPE *cm = costarr;
        if (isROS)			// ROS-type cost array
        {
            for (int i=0; i<ny; i++)
            {
                int k=i*nx;
                for (int j=0; j<nx; j++, k++, cmap++, cm++)
                {
                    *cm = COST_OBS;
                    int v = *cmap;
                    if (v < COST_OBS_ROS)
                    {
                        v = COST_NEUTRAL+COST_FACTOR*v;
                        if (v >= COST_OBS)
                            v = COST_OBS-1;
                        *cm = v;
                    }
                    else if(v == COST_UNKNOWN_ROS && allow_unknown)
                    {
                        v = COST_OBS-1;
                        *cm = v;
                    }
                }
            }
        }
        else				// not a ROS map, just a PGM
        {
            for (int i=0; i<ny; i++)
            {
                int k=i*nx;
                for (int j=0; j<nx; j++, k++, cmap++, cm++)
                {
                    *cm = COST_OBS;
                    if (i<7 || i > ny-8 || j<7 || j > nx-8)
                        continue;	// don't do borders
                    int v = *cmap;
                    if (v < COST_OBS_ROS)
                    {
                        v = COST_NEUTRAL+COST_FACTOR*v;
                        if (v >= COST_OBS)
                            v = COST_OBS-1;
                        *cm = v;
                    }
                    else if(v == COST_UNKNOWN_ROS)
                    {
                        v = COST_OBS-1;
                        *cm = v;
                    }
                }
            }

        }
    }

    void PathPlan::setGoal(const State& goal) {
        this->goal = goal;
    }

    void PathPlan::setStart(const State& start) {
        this->start = start;
    }

    vector<State> PathPlan::calcPath() {
        vector<PathItem> items;
        set<DiscreteState> visited;
        priority_queue<QPair, vector<QPair>, greater<QPair>> q;

        PathItem p0 = {start, 0, heuristic(start), 0, -1};
        visited.insert(discretize(start));
        q.push(QPair(p0.g+p0.h, p0.index));

        bool found_solution = false;
        PathItem goal_item;

        while(!q.empty()) {
            QPair qp = q.top();
            q.pop();
            PathItem p = items[qp.second];

            if(isGoalReached(p.state)) {
                found_solution = true;
                goal_item = p;
            }

            for(float t: steerings) {
                PathItem p1 = next(p, t);
                auto dp1 = discretize(p1.state);
                if (visited.count(dp1)) {
                    break;
                }

                p1.index = items.size();
                items.push_back(p1);
                visited.insert(dp1);
                q.push(QPair(p1.g + p1.h, p1.index));
            }
        }

        vector<State> sol;
        if (found_solution) {
            // backtrace path
            int i = goal_item.index;
            while (i >= 0) {
                sol.push_back(items[i].state);
                i = items[i].prev_index;
            }
            reverse(sol.begin(), sol.end());
        }
        return sol;
    }

    PathItem PathPlan::next(const PathItem& p, float t) {
        const State& s0 = p.state;
        PathItem p1;
        float dx = step * cos(s0[2]);
        float dy = step * sin(s0[2]);
        p1.state[0] = s0[0] + dx;
        p1.state[1] = s0[1] + dy;
        p1.state[2] = s0[2] + t;
        DiscreteState ds1 = discretize(p1.state);
        float cost = costarr[ds1[1]*nx + ds1[0]];
        p1.g = p.g + cost;
        p1.h = heuristic(p1.state);
        p1.prev_index = p.index;
        return p1;
    }

    inline float sqr(float x) { return x*x;}

    float PathPlan::distToGoal(const State& s) {
        float dist = sqrt(sqr(goal[0]-s[0]) + sqr(goal[1]-s[1]));
        return dist;
    }

    float PathPlan::heuristic(const State& s) {
        float d = distToGoal(s);
        float h = (d / step) * COST_NEUTRAL;
        return h;
    }

    float PathPlan::isGoalReached(const State& s) {
        float d = distToGoal(s);
        return (d < TOLERANCE);
    }

    DiscreteState PathPlan::discretize(const State& s) {
        DiscreteState ds(3);
        ds[0] = int(s[0]);
        ds[1] = int(s[1]);
        ds[2] = int(s[2] / D_YAW);
        return ds;
    }
}