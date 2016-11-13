#include "model.h"

#include <libconfig.h++>

#include <fstream>

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

void report_results(const std::vector<size_t> &hist_data,
		    const std::vector<double> &p, const size_t opt_count)
{
  std::cout.precision(5);
  std::cout.width(5);
  for (size_t i = 0; i < p.size(); ++i) {
    double exp_p = (double)hist_data[i]/opt_count;
    std::cout << "Expected probability: " << p[i]
	      << "\tExperimental probability: "
	      << exp_p << "\tError: "
	      << std::fabs((p[i] - exp_p)) * 100 / p[i] << "%\n";
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

void test_sample_generator(const std::vector<double> &p,
			   const size_t sample_size, const size_t opt_count)
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

void generate_task_list(double lambda_scheduling, double lambda_execution, size_t n,
			std::list<Event *> &calendar, Queue &queue,
			CS &cs, Time &system_time, unsigned int memory_size,
			unsigned int max_cores_num)
{
  std::vector<double> memory_distribution = {0.0, 0.02, 0.01, 0.01, 0.06, 0.1, 0.15,
					     0.15, 0.12, 0.08, 0.07, 0.07, 0.06, 0.06, 0.04};
  std::vector<double> cores_distribution = {0.0, 0.01, 0.15, 0.1, 0.1, 0.35, 0.05, 0.01, 0.01, 0.01, 0.01, 0.01,
					    0.01, 0.01, 0.01, 0.1, 0.01, 0.01, 0.01, 0.01, 0.01, 0.005, 0.005};

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
    if (n_cores > max_cores_num) {
      n_cores = max_cores_num;
    }

    memory = random_index(cum_memory_distribution);
    if (memory > memory_size) {
      memory = memory_size;
    }

    event = new Task(schedule_time, duration, n_cores, memory);
    schedule(event, &calendar, &queue, &cs, &system_time);
  }
}

int main(int argc, char *argv[])
{
  std::string conf_fname;
  if (argc == 1) {
    std::cerr << "Please, specify name of configuration file\n";
    return EXIT_FAILURE;
  } else {
    conf_fname = std::string(argv[1]);
  }

  libconfig::Config cfg;
  try {
    cfg.readFile(conf_fname.c_str());
  }
  catch (const libconfig::FileIOException &fioex) {
    std::cerr << "I/O error while reading file " << conf_fname << std::endl;
    return EXIT_FAILURE;
  }
  catch (const libconfig::ParseException &pex) {
    std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
	      << " - " << pex.getError() << std::endl;
    return EXIT_FAILURE;
  }

  std::string output_queue_stat("queue.dat");
  std::string output_cs_stat("cs.dat");

  size_t opt_count = 10000;

  unsigned int memory_size = 1024 * 1024 * 1024;
  unsigned int cores_count = 25;
  unsigned int queue_size = 300;

  //const libconfig::Setting &root = cfg.getRoot();
  try {
    memory_size = cfg.lookup("nmemory");
    std::cout << memory_size << std::endl;
  }
  catch(const libconfig::SettingNotFoundException &nfex) {
    std::cerr << "No 'nmemory' setting in configuration file" << std::endl;
  }

  CS cs(memory_size, cores_count);
  std::list<Event *> calendar;
  Time system_time = 0;
  Queue queue;

  srand(time(NULL));
  generate_task_list(0.35, 2.2, 250, calendar, queue, cs, system_time,
		     memory_size, cores_count);
  std::cout << "Scheduled time | execution_time | n_cores | memory |\n";
  for (auto iter = begin(calendar); iter != end(calendar); ++iter) {
    std::cout << std::setw(strlen("Scheduled time ")) << ((Task *)(*iter))->time() << std::setw(1) << '|'
	      << std::setw(strlen(" execution_time ")) << ((Task *)(*iter))->execution_time << std::setw(1) << '|'
	      << std::setw(strlen(" n_cores ")) << ((Task *)(*iter))->n_cores << std::setw(1) << '|'
	      << std::setw(strlen(" memory ")) << ((Task *)(*iter))->memory << std::setw(1) << '|' << std::endl;
  }
  simulate(&calendar, &queue, &cs, &system_time);
  std::vector<Time> stat = queue.get_stat();
  std::vector<CS::State> cs_stat = cs.get_stat();

  std::ofstream ost;
  ost.open(output_queue_stat.c_str());
  for (size_t i = 0; i < stat.size(); ++i) {
    ost << stat[i] << '\n';
  }
  ost.close();

  ost.open(output_cs_stat.c_str());
  for (size_t i = 0; i < cs_stat.size(); ++i) {
    ost << cs_stat[i].time << ' ' << cs_stat[i].free_cores << ' '
	<< cs_stat[i].free_memory << '\n';
  }
  ost.close();

  system("Rscript plot.R");

  std::cout << "Time at the end of simulation: " << system_time << std::endl;
  return 0;
}
