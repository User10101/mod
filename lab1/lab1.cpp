#include "model.h"

inline bool double_eq(double d1, double d2, double epsilon = 0.00001)
{
  return fabs(d1 - d2) < epsilon ? true : false;
}

inline double random_norm()
{
  return (double)rand() / (double)RAND_MAX;
}

std::vector<double> distr_to_cum(const std::vector<double> &distribution)
{
  std::vector<double> cum_prob(distribution.size());
  cum_prob[0] = distribution[0];
  for (size_t i = 1; i < distribution.size(); ++i) {
    cum_prob[i] = cum_prob[i - 1] + distribution[i];
  }

  return cum_prob;
}

size_t random_index(const std::vector<double> &cum_probabilities)
{
  if (!double_eq(cum_probabilities[cum_probabilities.size() - 1], 1.)) {
    throw std::invalid_argument("Sum of prob is " + std::to_string(cum_probabilities[cum_probabilities.size() - 1])
				+ " and not equals to one.");
  }
  
  if (cum_probabilities.size() == 1 && cum_probabilities[0] == 1.) {
    return 0;
  }
  
  double csi = random_norm();
  for (size_t i = 0; i < cum_probabilities.size(); ++i) {
    if (csi < cum_probabilities[i]) {
      return i;
    }
  }
}

std::vector<size_t> sample_nr(const std::vector<double> &prob, const size_t sample_size)
{
  std::vector<double> lprob(prob);
  std::vector<size_t> indexes(prob.size());
  std::vector<size_t> sample(sample_size);
  double csi, cum_prob, norm_mult = 1.;

  for (size_t i = 0; i < indexes.size(); ++i) {
    indexes[i] = i;
  }
  
  for (size_t i = 0; i < sample_size; ++i) {
    csi = random_norm() * norm_mult;
    cum_prob = .0;
    size_t j = 0;
    for ( ; csi > cum_prob + lprob[j]; ++j) {
      cum_prob += lprob[j];
    }
    sample[i] = indexes[j];
    norm_mult -= lprob[j];
    std::swap(lprob[j], lprob[lprob.size()-i-1]);
    std::swap(indexes[j], indexes[indexes.size()-i-1]);
  }

  return sample;
}

// Inverse cumulative distribution function - sin.
class Icdf_sin
{
public:
  double operator()(double csi) const;
};

double Icdf_sin::operator()(double csi) const
{
  return std::acos(1 - 2*csi);
}

template <typename T>
double random_distr(const T &distr)
{
  return distr(random_norm());
}

void report_results(const std::vector<size_t> &hist_data, const std::vector<double> &p, const size_t opt_count)
{
  std::cout.precision(5);
  std::cout.width(5);
  for (size_t i = 0; i < p.size(); ++i) {
    double exp_p = (double)hist_data[i]/opt_count;
    std::cout << "Expected probability: " << p[i] << "\tExperimental probability: "
	      << exp_p << "\tError: " << std::fabs((p[i] - exp_p)) * 100 / p[i] << "%\n";
  }
}

void test_number_generator(const std::vector<double> &p, const size_t opt_count)
{
  std::vector<double> cum_prob = distr_to_cum(p);
  std::vector<size_t> hist_data(p.size());
  for (size_t i = 0; i < opt_count; ++i) {
    hist_data[random_index(cum_prob)]++;
  }

  report_results(hist_data, p, opt_count);
}

void test_sample_generator(const std::vector<double> &p, const size_t sample_size, const size_t opt_count)
{
  std::vector<size_t> sample;
  std::vector<size_t> hist_data(p.size());
  for (size_t i = 0; i < opt_count; ++i) {
    sample = sample_nr(p, sample_size);
    std::set<size_t> used_indexes;
    for (size_t j = 0; j < sample_size; j++) {
      hist_data[sample[j]]++;
      if (used_indexes.find(sample[j]) != used_indexes.end()) {
	std::cout << "Повторение\n";
	return;
      }
      used_indexes.insert(sample[j]);
    }
  }

  report_results(hist_data, p, opt_count);
}

double exp_distr(double lambda)
{
  return -log(1 - random_norm()) / lambda;
}

void test_exp_generator(double lambda, const size_t opt_count)
{
  std::ofstream ost;
  ost.open("hist.dat", std::ios_base::trunc);
  for (size_t i = 0; i < opt_count; ++i) {
    ost << exp_distr(lambda) << '\n';
  }
  ost.close();

  system("Rscript plot_hist.R");
}

CS::CS(unsigned int memory, unsigned int cores)
  : memory_size(memory), core_num(cores)
{
  free_memory = memory_size;
  free_cores = core_num;
}

Op_result CS::mem_alloc(unsigned int size)
{
  if (free_memory >= size) {
    free_memory -= size;
    return SUCCESS;
  }

  return ERROR;
}

Op_result CS::mem_free(unsigned int size)
{
  if (free_memory + size <= memory_size) {
    free_memory += size;
    return SUCCESS;
  }

  return ERROR;
}

Op_result CS::cores_alloc(unsigned int num)
{
  if (free_cores >= num) {
    free_cores -= num;
    return SUCCESS;
  }

  return ERROR;
}

Op_result CS::cores_free(unsigned int num)
{
  if (free_cores + num <= core_num) {
    free_cores += num;
    return SUCCESS;
  }
  
  return ERROR;
}

Op_result schedule(Event *e)
{
  if (e->time < system_time) {
    return ERROR;
  }

  for (auto iter = begin(calendar); iter != end(calendar); ++iter) {
    if ((*iter)->time < e->time) {
      calendar.insert(iter, e);
      return SUCCESS;
    }
  }

  calendar.push_back(e);
  return SUCCESS;
}

Op_result cancel(Event *e, Time t)
{

}

void simulate()
{
  for (auto event : calendar) {
    event->execute();
    calendar.remove(*event);
  }
}

Task::Task(Time stime, Time etime, unsigned short ncores, unsigned short mem)
  : Event(stime), execution_time(etime), n_cores(ncores), memory(mem)
{
  // DO NOTHING
}

Task::~Task()
{
  // DO NOTHING
}

void Task::execute()
{
  // TODO 
}

Cancel_task::Cancel_task(Time stime, unsigned short ncores, unsigned short mem)
  : Event(stime), n_cores(ncores), memory(mem)
{
  // DO NOTHING
}

Cancel_task::~Cancel_task()
{
  // DO NOTHING
}

void Cancel_task::execute()
{
  // TODO
}

void generate_task_list(double lambda_scheduling, double lambda_execution, size_t n)
{
  std::vector<double> memory_distribution = {0.0, 0.02, 0.01, 0.01, 0.06, 0.1, 0.15,
					     0.15, 0.12, 0.08, 0.07, 0.07, 0.06, 0.06, 0.04};
  std::vector<double> cores_distribution = {0.0, 0.35, 0.15, 0.1, 0.1, 0.1, 0.05, 0.01, 0.01, 0.01, 0.01, 0.01,
					    0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.005, 0.005};

  std::vector<double> cum_memory_distribution = distr_to_cum(memory_distribution);
  std::vector<double> cum_cores_distribution = distr_to_cum(cores_distribution);

  Event *event;
  Time schedule_time = .0, duration;
  unsigned int n_cores;
  unsigned int memory;
  for (size_t i = 0; i < n; ++i) {
    schedule_time += exp_distr(lambda_scheduling);
    duration = exp_distr(lambda_execution);
    n_cores = random_index(cum_cores_distribution);
    memory = random_index(cum_memory_distribution);
    event = new Task(schedule_time, duration, n_cores, memory);
    schedule(event);
  }
}

int main(int argc, char *argv[])
{
  size_t opt_count = 10000;
  if (argc == 2) {
    opt_count = atoi(argv[1]);
  }
  
  srand(time(NULL));
  generate_task_list(0.35, 0.5, 10);
  std::cout << "Scheduled time | execution_time | n_cores | memory\n";
  for (auto iter = begin(calendar); iter != end(calendar); ++iter) {
    std::cout << ((Task *)(*iter))->time << ' ' << ((Task *)(*iter))->execution_time << ' ' << ((Task *)(*iter))->n_cores << ' ' << ((Task *)(*iter))->memory << std::endl;
  }
  return 0;
}
