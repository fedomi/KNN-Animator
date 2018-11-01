// C++ program to find groups of unknown 
// Points using K nearest neighbour algorithm. 
#include "stdafx.h"
#include<stdlib.h>
#include <math.h>
#include <algorithm>
#include<iostream>
#include<vector>
#include<functional>
#include<fstream>
#include<string>
using namespace std;

struct Frame {
	float rotation;
	float speed;
	vector<float> data;
};

struct knnPair {
	int index;
	float dist;
};

const int N = 114;
const int K = 10;	// Number of nearest neighbors
const int T = 5;	// Number of transition nearest neighbors
const int frameN = 7381;
const int DISCARD = 15;
const float w1 = 0.0f;
const float w2 = 1.0f;
const float TRANSITION_THRESHOLD = 1;
const float HIGHSPEED_THRESHOLD = 7;
const float LOWSPEED_THRESHOLD = 1;

string input_file = "Reduced20min.csv";
string output_file = "ReducedData.csv";

const float weights[N] = { 0.6,0.6,0.6,0.6,0.6,0.6,0.6,0.6,0.6,0.1,0.1,
0.1,0.6,0.6,0.6,0.3,0.3,0.3,0.1,0.1,0.1,0.3,0.3,0.3,0.3,0.3,
0.3,0.1,0.1,0.1,0.6,0.6,0.6,0.3,0.3,0.3,0.1,0.1,0.1,0.6,0.6,
0.6,0.6,0.6,0.6,0.1,0.1,0.1,0.6,0.6,0.6,0.6,0.6,0.6,0.1,0.1,
0.1,0.6,0.6,0.6,0.6,0.6,0.6,0.6,0.6,0.6,0.1,0.1,0.1,0.6,0.6,
0.6,0.3,0.3,0.3,0.1,0.1,0.1,0.3,0.3,0.3,0.3,0.3,0.3,0.1,0.1,
0.1,0.6,0.6,0.6,0.3,0.3,0.3,0.1,0.1,0.1,0.6,0.6,0.6,0.6,0.6,
0.6,0.1,0.1,0.1,0.6,0.6,0.6,0.6,0.6,0.6,0.1,0.1,0.1 };

#pragma region Function identifiers

void SplitString(const std::string& str, vector<string>& cont, char delim);
vector<float> ParseLineValues(vector<string> s);
vector<float> CalculateAdditionalRotationData(vector<float> currentFrame, vector<float> prevFrame);

#pragma endregion


bool comparison(const knnPair &a, const knnPair &b)
{
	return (a.dist < b.dist);
}

/* Compares the poses and rotation differences of both frames and returns a distance measure on that data */
float PoseDistance(Frame a, Frame b)
{
	float dist = 0;

	for (int i = 3; i < N+3; i++) {
		dist += abs(a.data[i] - b.data[i]) * weights[i];
	}

	return dist;
}

float MotionDistance(Frame a, Frame b) {
	
	float dist = 0;
	
	dist += w1 * abs(b.rotation - a.rotation) + w2 * abs(b.speed - a.speed);

	return dist;

}



void insert(vector<knnPair> &cont, knnPair value) {
	vector<knnPair>::iterator it = std::lower_bound(cont.begin(), cont.end(), value, comparison); // find proper position in descending order
	cont.insert(it, value); // insert before iterator it
}

vector<vector<int>> AllFramesKNN(vector<Frame> frames) {
	vector<int> neigh;
	vector<knnPair> pairs;
	vector<knnPair> highSpeedPairs;
	vector<knnPair> lowSpeedPairs;
	vector<vector<int>> outputArray;

	float tmpDis;

	for (int i = 0; i < frames.size(); i++) {
		pairs = {};
		highSpeedPairs = {};
		lowSpeedPairs = {};
		for (int j = 0; j < frames.size(); j++) {
			if (i != j && (j > i || j < (i - DISCARD) )) {
				tmpDis = PoseDistance(frames[i], frames[j]);
				if (tmpDis != 0) {
					knnPair p = { j, tmpDis };
					//pairs.push_back(p);
					//std::sort(pairs.begin(), pairs.end(), comparison);
					insert(pairs, p);	// sorted insert
					float modist = MotionDistance(frames[i], frames[j]);
					if (modist > TRANSITION_THRESHOLD) {
						if(frames[j].speed < LOWSPEED_THRESHOLD)
							insert(lowSpeedPairs, p);
						else if (frames[j].speed > HIGHSPEED_THRESHOLD)
							insert(highSpeedPairs, p);
					}
						
				}
				
			}
			//cout << i << " - " << j << endl;
			//if (j == 3034)
				//cout << "breakpoint here" << endl;
		}
		//cout << pairs[0].dist << " < " << pairs[1].dist << " < " << pairs[2].dist << endl;
		cout << i << endl;
		cout << pairs.size() << "-" << lowSpeedPairs.size() << "-" << highSpeedPairs.size() << endl;
		neigh = {};
		for (int n = 0; n < K; n++) {

			neigh.push_back(pairs[n].index);
			
		}
		int LS = lowSpeedPairs.size();
		if(LS > 0)
			for (int n = 0; n < min(T,LS); n++) {

				neigh.push_back(lowSpeedPairs[n].index);

			}

		int HS = highSpeedPairs.size();
		if (HS > 0)
			for (int n = 0; n < min(T, HS); n++) {
				neigh.push_back(highSpeedPairs[n].index);
			}


		//cout << neigh[0] << " " << neigh[1] << " " << neigh[2] << endl;
		//char c;
		//cin >> c;
		outputArray.push_back(neigh);

	}

	return outputArray;
}

#pragma region Python KNN
/*
for i in range(0, len(X)):
		dis = []
		for j in range(0, len(X)):
			if j == i:
				continue
			tmpDis = float(distance.euclidean(X[i], X[j]))
			if tmpDis == 0.0:
				continue
			dis.append([j, tmpDis])
			dis.sort(key=operator.itemgetter(1))
		nearIndex = []
		for n in range(k):
			nearIndex.append(dis[n][0])
		outputArray.append(nearIndex)
		print(i)
*/

#pragma endregion

#pragma region File related functions

vector<Frame> readFile() {
	ifstream input(input_file); //put your program together with this file in the same folder.
	vector<Frame> frames;
	vector<string> stringValues;
	vector<float> additionalData;
	char delim = ',';
	int count = 0;

	if (input.is_open()) {
		cout << "Reading file..." << endl;
		while (!input.eof()) {
			string line;
			//int data;
			stringValues = {};
			getline(input, line); //read number
			SplitString(line, stringValues, delim);

			Frame f;
			f.data = ParseLineValues(stringValues);

			if (count > 0) {
				additionalData = {};
				additionalData = CalculateAdditionalRotationData(f.data, frames[count - 1].data);
				frames[count - 1].data.insert(frames[count - 1].data.end(), additionalData.begin(), additionalData.end());
				//cout << frames[count - 1].data[0] << endl;
				frames[count - 1].rotation = f.data[4] - frames[count - 1].data[4];
				frames[count - 1].speed = sqrt((f.data[0] - frames[count - 1].data[0])*(f.data[0] - frames[count - 1].data[0])
					+ (f.data[2] - frames[count - 1].data[2])*(f.data[2] - frames[count - 1].data[2]));

			}
			if (count < frameN)
				frames.push_back(f);
			else if (count == frameN)
				break;
			//cout << f.data[0] << endl; //print it out
			count++;
		}

		cout << "File read" << endl;
	}
	else {
		cout << "File not found " << endl;
	}

	return frames;
}

vector<float> CalculateAdditionalRotationData(vector<float> currentFrame, vector<float> prevFrame) {
	vector<float> result;

	for (int i = 3; i < currentFrame.size(); i++) {
		result.push_back(currentFrame[i] - prevFrame[i]);
	}

	return result;
}

void PrintFile(vector<Frame> frames, vector<vector<int>> knn) {
	ofstream myfile(output_file);
	if (myfile.is_open())
	{
		for (int i = 0; i < frames.size(); i++) {
			myfile << i << "," << frames[i].rotation << "," << frames[i].speed;
			for (int j = 0; j < K + T; j++) {
				myfile << "," << knn[i][j];
			}
			myfile << endl;
		}

		myfile.close();
	}

}

void SplitString(const std::string& str, vector<string>& cont, char delim = ',')
{
	std::size_t current, previous = 0;
	current = str.find(delim);


	while (current != std::string::npos) {
		cont.push_back(str.substr(previous, current - previous));
		previous = current + 1;
		current = str.find(delim, previous);
	}
	cont.push_back(str.substr(previous, current - previous));
}

vector<float> ParseLineValues(vector<string> s) {
	vector<float> result;

	for (int i = 0; i < s.size(); i++) {
		result.push_back(atof(s[i].c_str()));
	}

	return result;
}

#pragma endregion


// Driver code 
int main()
{
	vector<Frame> frames = readFile();
	cout << frames.size() << endl;
	vector<vector<int>> knnResult = AllFramesKNN(frames);
	PrintFile(frames, knnResult);
	char c;
	cin >> c;

	return 0;
}
