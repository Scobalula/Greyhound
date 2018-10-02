#pragma once

#include <cstdint>
#include <memory>
#include <thread>
#include <vector>

// A class that handles parallel operations
class CoDXConverter
{
public:

	// Spawns a new converter, then waits for it to finish execution before continuing
	CoDXConverter(const std::function<void(void)> Job, uint32_t DegreeOfParallelism)
	{
		// The workers for this instance
		std::vector<std::thread> Workers;
		
		// Loop and add the jobs
		for (uint32_t i = 0; i < DegreeOfParallelism; i++)
		{
			// Add the job, start the thread
			Workers.emplace_back(Job);
		}

		// Wait for all workers to end
		for (auto& Worker : Workers)
		{
			// Wait for the worker
			try
			{
				// Join it
				Worker.join();
			}
			catch (...)
			{
				// Nothing, thread already exited
			}
		}
	}
};