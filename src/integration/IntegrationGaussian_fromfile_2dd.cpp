#include "../parameters/ParamParser_getopt.hpp"
#include "../integration/IntegrationGaussian2D.hpp"
#include "../io/fileIO.hpp"
#include <chrono>
#include <fstream>

#include "runIntegration.hpp"

#ifdef CIMG_ON
#include "../../externals/CImg-1.6.9/CImg.h"
#endif

#define D 2

using namespace utk;

int main(int argc, char** argv)
{
	ParamParser_getopt parser;
	IntegrationGaussian2D gauss;

	double param_stddev_x = 0.15;
	double param_mean_x = 0.5;
	double param_stddev_y = 0.15;
	double param_mean_y = 0.5;

	bool param_output_func=false;
	parser.addLongOption((char*)"stddevx", &param_stddev_x, 1, assignDouble, displayDouble, (char*)"<double=0.15>\tSets the x value for the standard deviation of the gaussian", (char*)"Stddev X");
	parser.addLongOption((char*)"meanx", &param_mean_x, 1, assignDouble, displayDouble, (char*)"<double=0.5>\tSets the x value for the mean of the gaussian", (char*)"Mean X");
	parser.addLongOption((char*)"stddevy", &param_stddev_y, 1, assignDouble, displayDouble, (char*)"<double=0.15>\tSets the y value for the standard deviation of the gaussian", (char*)"Stddev Y");
	parser.addLongOption((char*)"meany", &param_mean_y, 1, assignDouble, displayDouble, (char*)"<double=0.5>\tSets the y value for the mean of the gaussian", (char*)"Mean Y");
	
	parser.addLongOption((char*)"outputfunc", &param_output_func, 0, assignBoolTrue, displayBool, (char*)"Outputs the integrated function in func.dat (plottable with gnuplot)", (char*)"Output Func");
#ifdef CIMG_ON
	using namespace cimg_library;
	bool param_output_img=false;
	parser.addLongOption((char*)"outputimg", &param_output_img, 0, assignBoolTrue, displayBool, (char*)"Outputs the integrated function in func.png", (char*)"Output Img");
#endif
	
	initParserIntegration(&parser);
	
	parser.parse(argc, argv);
	
	if(!dealParamParserIntegration(&parser))
		return 0;
	
	gauss.setStddevX(param_stddev_x);
	gauss.setMeanX(param_mean_x);
	gauss.setStddevY(param_stddev_y);
	gauss.setMeanY(param_mean_y);

	PointsetReader< D, double, Point<D, double> > stream;
	stream.open(param_input);
	
	IntegrationStatistics stats;

	int size = 0;
	Pointset< D, double, Point<D, double> > pts;
	while(stream.readPointset(pts))
	{
		if(param_npts != 0)
			pts.resize(param_npts);
		
		if(stats.nbpts == 0)
			stats.nbpts = pts.size();
		else if(stats.nbpts != pts.size())
			WARNING("File contains pointsets of different sizes, averaging");
		
		double analytical = 0;
		double integration = 0;
		if(!gauss.compute< D, double, Point<D, double> >(pts, integration, analytical))
			return 1;
		
		stats.setAnalytical(analytical);
					
		if(param_brute)
		{
			if(param_output.empty())
				std::cout << pts.size() << " " << integration << std::endl;
			else
			{
				std::ofstream file(param_output, std::ios::out);
				file << pts.size() << " " << integration << std::endl;
				file.close();
			}
		}
		else
			stats.addValue(integration);
		
		size+=pts.size();
		pts.clear();
	}
	
	stats.nbpts = size/stats.iter;
	
	if(!param_brute)
	{
		stats.computeStatistics();
		
		if(param_output.empty())
			std::cout << stats << std::endl;
		else
		{
			std::ofstream file(param_output, std::ios::out);
			file << stats << std::endl;
			file.close();
		}
	}
	
	if(param_output_func)
	{
		std::ofstream file("func.dat");
		
		int res=40;
		for(int i=0; i<res; i++)
		for(int j=0; j<res; j++)
		{
			Point<2, double> pt; pt.pos()[0] = (double)i/res; pt.pos()[1] = (double)j/res;
			file << i << " " << j << " " << gauss.sampleIntegrand(pt) << std::endl;
		}
		
		file.close();
	}
#ifdef CIMG_ON
	if(param_output_img)
	{
		CImg<double> m_image_rgb(1024, 1024, 1, 3);
		
		for(double i=0; i<=1; i+=1.0/1024.0)
		{
			for(double j=0; j<=1; j+=1.0/1024.0)
			{
				Point<2, double> pt; pt.pos()[0] = i; pt.pos()[1] = j;
				m_image_rgb.atXYZC(i*1024, j*1024, 0, 0) = gauss.sampleIntegrand(pt)*255;
				m_image_rgb.atXYZC(i*1024, j*1024, 0, 1) = gauss.sampleIntegrand(pt)*255;
				m_image_rgb.atXYZC(i*1024, j*1024, 0, 2) = gauss.sampleIntegrand(pt)*255;
			}
		}
		
		m_image_rgb.save("func.png");
	}
#endif
	
	return 0;
}
