#include "cpuwatch.h"

CPUWatch::CPUWatch(boost::shared_ptr<Config> cfg) : cfg(cfg)
{
    this->watch = true;
}

void CPUWatch::queryCPUProc()
{
    try
    {
        while (watch)
        {
            this->cpuAVGLoadStream.open(boost::filesystem::path(this->cfg->procCpuAvgLoad));
            if (this->cpuAVGLoadStream.is_open())
            {
                string line;

                this->cpuAVGLoadStream.close();
                boost::this_thread::sleep(boost::posix_time::milliseconds(this->cfg->waitCpuThread));
            } else {
                cerr << "Can not open: " << this->cfg->procCpuAvgLoad << endl;
            }
        }
    }
    catch(boost::thread_interrupted)
    {
        cout << "CPU Watch thread interrupted" << endl;
    }
}
