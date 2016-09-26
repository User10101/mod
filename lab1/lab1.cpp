#include "model.h"

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
  if (cum_probabilities[cum_probabilities.size() - 1] != 1.) {
    throw std::invalid_argument("Sum of prob is not equals to one.");
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
  double cum_sum = .0;
  std::vector<double> generated_values(opt_count);
  for (size_t i = 0; i < opt_count; ++i) {
    generated_values[i] = exp_distr(lambda);
    cum_sum += generated_values[i];
  }

  Gnuplot gp;
  gp << "set boxwidth 0.28 absolute;\n";
  gp << "set style fill solid 1.0 border 0.5;\n";
  gp << "bin_width = 0.3;\n";
  gp << "bin_number(x) = floor(x / bin_width);\n";
  gp << "rounded(x) = bin_width * (bin_number(x));\n";
  gp << "plot" << gp.file1d(generated_values) << "using(rounded($1)):(1) smooth frequency with boxes title 'Count of experiments: "
     << opt_count << "';\n";
}

Task::Task()
{

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

int main(int argc, char *argv[])
{
  size_t opt_count = 10000;
  if (argc == 2) {
    opt_count = atoi(argv[1]);
  }
  
  srand(time(NULL));
  std::vector<double> p = {0.05, 0.05, 0.15, 0.05, 0.2, 0.25, 0.25};
  std::cout << "----- Number generator test -----\n";
  test_number_generator(p, opt_count);
  std::cout << "----- Sample generator test -----\n";
  test_sample_generator(p, 1, opt_count);
  std::cout << "----- Distribution test -----\n";
  test_exp_generator(1.5, opt_count);
  std::cout << "Press any key to exit...\n";
  std::cin.get();
  return 0;
}
