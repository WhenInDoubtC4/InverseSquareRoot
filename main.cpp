#include <iostream>
#include <fstream>
#include <cmath>
#include <chrono>
#include <random>
#include <array>
#include <iostream>

#define SEPARATOR ","

using namespace std;
using namespace chrono;

const int testSize = 10000;

const size_t bigger_than_cachesize = 10 * 1024 * 1024;
long *p = new long[bigger_than_cachesize];

void flushCache()
{
	for(int i = 0; i < bigger_than_cachesize; i++)
	{
	   p[i] = rand();
	}
}

float fastInvSqrt(float input)
{
	int iota = *reinterpret_cast<int*>(&input);
	int sigma =  1597308761 - (iota >> 1);
	float result = *reinterpret_cast<float*>(&sigma);

	return result;
}

float fastInvSqrtN(float input)
{
	int iota = *reinterpret_cast<int*>(&input);
	int sigma =  1597308761 - (iota >> 1);
	float result = *reinterpret_cast<float*>(&sigma);
	result *= 1.5f - 0.5f * input * result * result;

	return result;
}

float invSqrt(float input)
{
	return 1.f / sqrt(input);
}

float invSqrt2(float input)
{
	return pow(input, -0.5f);
}

void timeCompare(float input, double& outFastRuntime, double& outFastNRuntime, double &outImpl1Runtime, double& outImpl2Runtime)
{
	auto fast_t0 = high_resolution_clock::now();
	fastInvSqrt(input);
	auto fast_t1 = high_resolution_clock::now();
	duration<double, micro> fastRuntime = fast_t1 - fast_t0;
	outFastRuntime = fastRuntime.count();

	flushCache();

	auto fastN_t0 = high_resolution_clock::now();
	fastInvSqrtN(input);
	auto fastN_t1 = high_resolution_clock::now();
	duration<double, micro> fastNRuntime = fastN_t1 - fastN_t0;
	outFastNRuntime = fastNRuntime.count();

	flushCache();

	auto impl1_t0 = high_resolution_clock::now();
	invSqrt(input);
	auto impl1_t1 = high_resolution_clock::now();
	duration<double, micro> impl1Runtime = impl1_t1 - impl1_t0;
	outImpl1Runtime = impl1Runtime.count();

	flushCache();

	auto impl2_t0 = high_resolution_clock::now();
	invSqrt2(input);
	auto impl2_t1 = high_resolution_clock::now();
	duration<double, micro> impl2Runtime = impl2_t1 - impl2_t0;
	outImpl2Runtime = impl2Runtime.count();
}

void acccuracyCompare(float input, float& outFastErr, float& outFastPercentageErr, float& outFastNErr, float& outFastNPercentageErr)
{
	float actual = invSqrt(input);
	float fastResult = fastInvSqrt(input);
	float fastNResult = fastInvSqrtN(input);

	outFastErr = actual - fastResult;
	outFastPercentageErr = (actual - fastResult) / actual;

	outFastNErr = actual - fastNResult;
	outFastNPercentageErr = (actual - fastNResult) / actual;
}

int main(int argc, char* argv[])
{
	random_device device;
	mt19937 generator(device());

	lognormal_distribution<float> distr(log(1e6), log(1e4));

	ofstream dataFile;
	dataFile.open("testResults.csv");

	//Table header
	dataFile << "Number" << SEPARATOR;
	dataFile << "Actual value" << SEPARATOR;
	dataFile << "Fast runtime" << SEPARATOR;
	dataFile << "FastN runtime" << SEPARATOR;
	dataFile << "Impl1 runtime" << SEPARATOR;
	dataFile << "Impl2 runtime" << SEPARATOR;
	dataFile << "Fast abs err" << SEPARATOR;
	dataFile << "Fast rel err" << SEPARATOR;
	dataFile << "FastN abs err" << SEPARATOR;
	dataFile << "FastN rel err" << SEPARATOR;
	dataFile << endl;

	bool firstRun = true;

	for (int i = 0; i < testSize + 1; i++)
	{
		cout << "Running test " << i << "/" << testSize + 1 << " (" << float(i) / float(testSize + 1) *100.f << "%)" << endl;

		float n = distr(generator);

		//Discard results on first fun (because of instruction caching)
		if (firstRun)
		{
			firstRun = false;
			continue;
		}

		dataFile << n << SEPARATOR; //Number
		dataFile << invSqrt(n) << SEPARATOR; //Actual value

		double fastRuntime;
		double fastNRuntime;
		double i1Runtime;
		double i2Runtime;
		timeCompare(n, fastRuntime, fastNRuntime, i1Runtime, i2Runtime);

		dataFile << fastRuntime << SEPARATOR;
		dataFile << fastNRuntime << SEPARATOR;
		dataFile << i1Runtime << SEPARATOR;
		dataFile << i2Runtime << SEPARATOR;

		float fastAbsErr;
		float fastRelErr;
		float fastNAbsErr;
		float fastNRelErr;
		acccuracyCompare(n, fastAbsErr, fastRelErr, fastNAbsErr, fastNRelErr);

		dataFile << fastAbsErr << SEPARATOR;
		dataFile << fastRelErr << SEPARATOR;
		dataFile << fastNAbsErr << SEPARATOR;
		dataFile << fastNRelErr << SEPARATOR;

		dataFile << endl;
	}

	dataFile.close();

	cout << "Test complete" << endl;

	return 0;
}
