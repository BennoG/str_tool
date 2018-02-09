
using namespace std;

namespace ansStl
{

	/* moving average class om van de laatste x waarde het gemiddelde te bepalen
	*/
	template <typename T,unsigned int N> class MovingAverage
	{
	private:
		T samples_[N];
		unsigned int num_samples_;
		T total_;
	public:
		MovingAverage():num_samples_(0),total_(0){memset(samples_,0,sizeof(samples_));}
		void operator()(T sample) { add(sample); }
		T & operator[](int idx)		// acces direct to members of the array
		{
			if ((idx >= 0) && (idx < N)) return samples_[(num_samples_ + N - (1 + idx)) % N];
			return samples_[0];
		}
		void add(T sample)
		{
			if (num_samples_ < N)
			{
				samples_[num_samples_++] = sample;
				total_ += sample;
			}
			else
			{
				T& oldest = samples_[num_samples_++ % N];
				total_ += sample - oldest;
				oldest = sample;
			}
		}
		ansStl::cST log()
		{
			ansStl::cST res;
			res.setf("MovingAverage(%d) num(%d) sum(%d) ",N,num_samples_,(int)total_);
			for (unsigned int i = 0; i < N; i++)
				res.append("V[%d]=%d ",i,(int)(samples_[(num_samples_ + N - (1 + i)) % N]));
			return res;
		}

		double avgf(){ return (total_ * 1.0 / ((int)max(1u,min(num_samples_,N)))); }// gemiddelde als floating point waarde	(unsigned van num_samples en N ging verkeerd)
		int avgr(){return ((int)(avgf() + 0.5));}							// gemiddelde als afgeronde integer
		T avg(){ return (total_ / ((int)max(1u,min(num_samples_,N)))); }			// gemiddelde als Type	(unsigned van num_samples en N ging verkeerd)
		operator T() const { return avg();}
	};

}

