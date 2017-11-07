/**********************************************************************

Copyright 2017 The University of Leeds

This file is part of the R humanleague package.

humanleague is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

humanleague is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

A copy of the GNU General Public License can be found in LICENCE.txt
in the project's root directory, or at <http://www.gnu.org/licenses/>.

**********************************************************************/


#include <Rcpp.h>
using namespace Rcpp;

#include "NDArrayUtils.h"
#include "Index.h"
#include "IPF.h"
#include "QIS.h"
#include "QISI.h"
#include "Integerise.h"
#include "StatFuncs.h"
#include "Sobol.h"

#include "QIWS.h" // TODO deprecate
#include "GQIWS.h" // TODO deprecate
#include "QSIPF.h" // TODO deprecate

#include "UnitTester.h"

#include <vector>

#include <cstdint>

//#include <csignal>

// // Handler for ctrl-C
// extern "C" void sigint_handler(int)
// {
//   // throw back to R
//   throw std::runtime_error("User interrupt");

// }
// Enable ctrl-C to interrupt the code
// TODO this doesnt seem to work, perhaps another approach (like a separate thread?)
//void (*oldhandler)(int) = signal(SIGINT, sigint_handler);


// void doSolveConstrained(List& result, IntegerVector dims, const std::vector<std::vector<uint32_t>>& m, const NDArray<2, bool>& permitted)
// {
//   CQIWS solver(m, permitted);
//   result["method"] = "QIWS-C";
//   result["conv"] = solver.solve();
//   result["chiSq"] = solver.chiSq();
//   std::pair<double, bool> pVal = solver.pValue();
//   result["pValue"] = pVal.first;
//   if (!pVal.second)
//   {
//     result["warning"] = "p-value may be inaccurate";
//   }
//   result["error.margins"] = std::vector<uint32_t>(solver.residuals(), solver.residuals() + 2);
//   const typename QIWS<2>::table_t& t = solver.result();
//
//   const NDArray<2, double>& p = solver.stateProbabilities();
//   Index<2, Index_Unfixed> idx(t.sizes());
//   IntegerVector values(t.storageSize());
//   NumericVector probs(t.storageSize());
//   while (!idx.end())
//   {
//     values[idx.colMajorOffset()] = t[idx];
//     probs[idx.colMajorOffset()] = p[idx];
//     ++idx;
//   }
//   values.attr("dim") = dims;
//   probs.attr("dim") = dims;
//   result["p.hat"] = probs;
//   result["x.hat"] = values;
// }

void doSolveGeneral(List& result, IntegerVector dims, const std::vector<std::vector<uint32_t>>& m, const NDArray<double>& exoProbs)
{
  GQIWS solver(m, exoProbs);
  result["method"] = "QIWS-G";
  result["conv"] = solver.solve();

  const typename QIWS::table_t& t = solver.result();
  //
  // const NDArray<2, double>& p = solver.stateProbabilities();
  Index idx(t.sizes());
  IntegerVector values(t.storageSize());
  // NumericVector probs(t.storageSize());
  while (!idx.end())
  {
    values[idx.colMajorOffset()] = t[idx];
  //   probs[idx.colMajorOffset()] = p[idx];
    ++idx;
  }
  values.attr("dim") = dims;
  // probs.attr("dim") = dims;
  // result["p.hat"] = probs;
  result["x.hat"] = values;
}

// void doSolveCorrelated(List& result, IntegerVector dims, const std::vector<std::vector<uint32_t>>& m, double rho)
// {
//   RQIWS solver(m, rho);
//   result["method"] = "QIWS-R";
//   result["conv"] = solver.solve();
//   result["chiSq"] = solver.chiSq();
//   std::pair<double, bool> pVal = solver.pValue();
//   result["pValue"] = pVal.first;
//   if (!pVal.second)
//   {
//     result["warning"] = "p-value may be inaccurate";
//   }
//   result["error.margins"] = std::vector<uint32_t>(solver.residuals(), solver.residuals() + 2);
//   const typename QIWS<2>::table_t& t = solver.result();
//
//   const NDArray<2, double>& p = solver.stateProbabilities();
//   Index<2, Index_Unfixed> idx(t.sizes());
//   IntegerVector values(t.storageSize());
//   NumericVector probs(t.storageSize());
//   while (!idx.end())
//   {
//     values[idx.colMajorOffset()] = t[idx];
//     probs[idx.colMajorOffset()] = p[idx];
//     ++idx;
//   }
//   values.attr("dim") = dims;
//   probs.attr("dim") = dims;
//   result["p.hat"] = probs;
//   result["x.hat"] = values;
// }

// void doConstrain(List& result, NDArray<uint32_t>& population, const NDArray<bool>& permitted)
// {
//   Constrain::Status status = CQIWS::constrain(population, permitted, population.storageSize());
//
//   switch(status)
//   {
//   case Constrain::SUCCESS:
//     result["conv"] = true;
//     break;
//   case Constrain::ITERLIMIT:
//     result["conv"] = false;
//     result["error"] = "iteration limit reached";
//     break;
//   case Constrain::STUCK:
//     result["conv"] = false;
//     result["error"] = "cannot adjust population: check validity of constraint";
//     break;
//   default:
//     result["conv"] = false;
//     result["error"] = "constrain algorithm status invalid. please report this bug";
//   }
//
//   IntegerVector dims;
//   dims.push_back(population.sizes()[0]);
//   dims.push_back(population.sizes()[1]);
//   Index<2, Index_Unfixed> idx(population.sizes());
//   IntegerVector values(population.storageSize());
//   size_t pop = 0;
//   while (!idx.end())
//   {
//     values[idx.colMajorOffset()] = population[idx];
//     pop += population[idx];
//     ++idx;
//   }
//   values.attr("dim") = dims;
//   result["x.hat"] = values;
// }



//' Generate a population in n dimensions given n marginals.
//'
//' Using Quasirandom Integer Without-replacement Sampling (QIWS), this function
//' generates an n-dimensional population table where elements sum to the input marginals, and supplemental data.
//' @param marginals a List of n integer vectors containing marginal data (2 <= n <= 12). The sum of elements in each vector must be identical
//' @return an object containing: the population matrix, the occupancy probability matrix, a convergence flag, the chi-squared statistic, p-value, and error value (nonzero if not converged)
//' @examples
//' synthPop(list(c(1,2,3,4), c(3,4,3)))
//' @export
// [[Rcpp::export]]
List synthPop(List marginals)
{
  const size_t dim = marginals.size();

  std::vector<std::vector<uint32_t>> m(dim);
  IntegerVector sizes;
  // TODO verbose flag? Rcout << "Dimension: " << dim << "\nMarginals:" << std::endl;
  for (size_t i = 0; i < dim; ++i)
  {
    const IntegerVector& iv = marginals[i];
    m[i].reserve(iv.size());
    std::copy(iv.begin(), iv.end(), std::back_inserter(m[i]));
    //Rcout << "[" << std::accumulate(m[i].begin(), m[i].end(), 0) << "] ";
    //print(m[i].data(), m[i].size(), m[i].size(), Rcout);
    sizes.push_back(iv.size());
  }
  List result;

  QIWS solver(m);

  result["method"] = "QIWS";
  result["conv"] = solver.solve();
  result["chiSq"] = solver.chiSq();
  std::pair<double, bool> pVal = solver.pValue();
  result["pValue"] = pVal.first;
  if (!pVal.second)
  {
    result["warning"] = "p-value may be inaccurate";
  }
  result["error.margins"] = solver.residuals();
  const typename QIWS::table_t& t = solver.result();
  const NDArray<double>& p = solver.stateProbabilities();

  // Rcpp::Rcout << t.storageSize() << std::endl;
  // print(t.sizes(), Rcpp::Rcout);

  IntegerVector values(t.storageSize());
  NumericVector probs(t.storageSize());
  for (Index idx(t.sizes()); !idx.end(); ++idx)
  {
    values[idx.colMajorOffset()] = t[idx];
    probs[idx.colMajorOffset()] = p[idx];
  }
  values.attr("dim") = sizes;
  probs.attr("dim") = sizes;
  result["p.hat"] = probs;
  result["x.hat"] = values;

  return result;
}

// [[Rcpp::export]]
List synthPopG(List marginals, NumericMatrix exoProbsIn)
{
  if (marginals.size() != 2)
    throw std::runtime_error("CQIWS invalid dimensionality: " + std::to_string(marginals.size()));

  std::vector<std::vector<uint32_t>> m(2);

  const IntegerVector& iv0 = marginals[0];
  const IntegerVector& iv1 = marginals[1];
  IntegerVector dims(2);
  dims[0] = iv0.size();
  dims[1] = iv1.size();
  m[0].reserve(dims[0]);
  m[1].reserve(dims[1]);
  std::copy(iv0.begin(), iv0.end(), std::back_inserter(m[0]));
  std::copy(iv1.begin(), iv1.end(), std::back_inserter(m[1]));

  if (exoProbsIn.rows() != dims[0] || exoProbsIn.cols() != dims[1])
    throw std::runtime_error("CQIWS invalid permittedStates matrix size");

  std::vector<int64_t> d{ dims[0], dims[1] };
  NDArray<double> exoProbs(d);

  for (d[0] = 0; d[0] < dims[0]; ++d[0])
  {
    for (d[1] = 0; d[1] < dims[1]; ++d[1])
    {
      exoProbs[d] = exoProbsIn(d[0],d[1]);
    }
  }

  List result;
  doSolveGeneral(result, dims, m, exoProbs);

  return result;
}


// //' IPF
// //'
// //' C++ IPF implementation
// //' @param seed an n-dimensional array of seed values
// //' @param marginals a List of n integer vectors containing marginal data. The sum of elements in each vector must be identical
// //' @return an object containing: ...
// //' @export
// // [[Rcpp::export]]
// List ipf(NumericVector seed, List marginals)
// {
//   const size_t dim = marginals.size();
//
//   Dimension sizes = seed.attr("dim");
//   std::vector<std::vector<double>> m(dim);
//   std::vector<int64_t> s(dim);
//
//   if (sizes.size() != dim)
//     throw std::runtime_error("no. of marginals not equal to seed dimension");
//
//   // insert marginals in reverse order (R being column-major)
//   for (size_t i = 0; i < dim; ++i)
//   {
//     const NumericVector& iv = marginals[i];
//     if (iv.size() != sizes[i])
//       throw std::runtime_error("seed-marginal size mismatch");
//     s[dim-i-1] = sizes[i];
//     m[dim-i-1].reserve(iv.size());
//     std::copy(iv.begin(), iv.end(), std::back_inserter(m[dim-i-1]));
//   }
//
//   // Storage for result
//   NumericVector r(sizes);
//
//   List result;
//   // Read-only shallow copy of seed
//   const NDArray<double> seedwrapper(s, (double*)&seed[0]);
//   // Do IPF (could provide another ctor that takies preallocated memory for result)
//   IPF ipf(seedwrapper, m);
//   // Copy result data into R array
//   const NDArray<double>& tmp = ipf.result();
//   std::copy(tmp.rawData(), tmp.rawData() + tmp.storageSize(), r.begin());
//   result["conv"] = ipf.conv();
//   result["result"] = r;
//   result["pop"] = ipf.population();
//   result["iterations"] = ipf.iters();
//   result["errors"] = ipf.errors();
//   result["maxError"] = ipf.maxError();
//   return result;
// }

template<typename T, typename R>
NDArray<T> convertRArray(R rArray)
{
  // workaround for 1-d arrays (which don't have "dim" attribute)
  //Dimension colMajorSizes(rArray.hasAttribute("dim") ? Dimension(rArray.attr("dim")) : Dimension((size_t)rArray.size()));
  Dimension colMajorSizes;
  if (rArray.hasAttribute("dim"))
  {
    colMajorSizes = rArray.attr("dim");
  }
  else
  {
    colMajorSizes = rArray.size();
  }
  const size_t dim = colMajorSizes.size();

  // Reverse sizes for row-major storage
  std::vector<int64_t> sizes(dim);
  for (size_t i = 0; i < sizes.size(); ++i)
  {
    //sizes[i] = colMajorSizes[dim-i-1]; // reversing the dims messes the tablular output
    sizes[i] = colMajorSizes[i];
  }

  NDArray<T> array(sizes);

  for (Index idx(array.sizes());!idx.end(); ++idx)
  {
    array[idx] = rArray[idx.colMajorOffset()];
  }
  //std::copy(&rArray[0], &rArray[0] + rArray.size(), const_cast<double*>(array.rawData()));

  return array;
}


//' Multidimensional IPF
//'
//' C++ multidimensional IPF implementation
//' @param seed an n-dimensional array of seed values
//' @param indices an array listing the dimension indices of each marginal as they apply to the seed values
//' @param marginals a List of arrays containing marginal data. The sum of elements in each array must be identical
//' @return an object containing: ...
//' @export
// [[Rcpp::export]]
List ipf(NumericVector seed, List indices, List marginals)
{
  if (indices.size() != marginals.size())
  {
    throw std::runtime_error("index and marginal lists are different lengths");
  }

  const int64_t k = marginals.size();

  Dimension rSizes = seed.attr("dim");
  int64_t dim = rSizes.size();

  std::vector<NDArray<double>> m;
  m.reserve(k);
  std::vector<std::vector<int64_t>> idx;
  idx.reserve(k);
  std::vector<int64_t> s;
  s.reserve(dim);

  if (indices.size() != marginals.size())
    throw std::runtime_error("no. of marginals not equal to no. of indices");

  // assemble dimensions (row major) for seed
  for (int64_t i = dim-1; i >= 0; --i)
    s.push_back(rSizes[(size_t)i]);

  // insert indices and marginals in reverse order (R being column-major)
  for (int64_t i = k-1; i >= 0; --i)
  {
    const IntegerVector& iv = indices[i];
    const NumericVector& nv = marginals[i];
    idx.push_back(std::vector<int64_t>(iv.size()));
    // also need to reverse dimension indices
    for (size_t j = 0; j < iv.size(); ++j)
      idx.back()[j] = dim - iv[j];
    //std::copy(iv.begin(), iv.end(), idx.back().begin());
    // convert to NDArray
    m.push_back(std::move(convertRArray<double, NumericVector>(nv)));
  }

  // Storage for result

  List result;
  // Read-only shallow copy of seed
  const NDArray<double> seedwrapper(s, (double*)&seed[0]);
  // Do IPF (could provide another ctor that takes preallocated memory for result)
  IPF<double> ipf(idx, m);
  NumericVector r(rSizes);
  // Copy result data into R array
  const NDArray<double>& tmp = ipf.solve(seedwrapper);
  std::copy(tmp.rawData(), tmp.rawData() + tmp.storageSize(), r.begin());
  result["conv"] = ipf.conv();
  result["result"] = r;
  result["pop"] = ipf.population();
  result["iterations"] = ipf.iters();
  //  result["errors"] = ipf.errors();
  result["maxError"] = ipf.maxError();
  return result;
}


// Helper to get overall dimension and sizes before constructing QIS
// (as indices need to be relabelled and marginals transposed to construct in row-major form)
std::vector<int64_t> dimensionHelper(List indices, List marginals)
{
  std::map<int64_t,int64_t> lookup;
  // dont worry about inconsistencies here, QIS will detect and report them
  for (size_t i = 0; i < indices.size(); ++i)
  {
    const IntegerVector& iv = indices[i];
    const IntegerVector& mv = marginals[i];
    std::vector<int64_t> colMajorSizes;
    if (mv.hasAttribute("dim"))
    {
      colMajorSizes = as<std::vector<int64_t>>(mv.attr("dim"));
    }
    else
    {
      colMajorSizes.push_back(mv.size());
    }
    for (size_t j = 0; j < colMajorSizes.size(); ++j)
    {
      lookup[iv[j]] = colMajorSizes[j];
    }
  }
  std::vector<int64_t> ret;
  ret.reserve(lookup.size());
  for (const auto& kv: lookup)
    ret.push_back(kv.second);
  return ret;
}

//' Multidimensional QIS
//'
//' C++ multidimensional Quasirandom Integer Sampling implementation
//' @param indices an array listing the dimension indices of each marginal as they apply to the seed values
//' @param marginals a List of arrays containing marginal data. The sum of elements in each array must be identical
//' @return an object containing: ...
//' @export
// [[Rcpp::export]]
List qis(List indices, List marginals, int skips = 0)
{
  if (indices.size() != marginals.size())
  {
    throw std::runtime_error("index and marginal lists are different lengths");
  }

    // we need the overall dimension and sizes upfront to assemble the problem in row-major rather than col-major form.
  std::vector<int64_t> rSizes = dimensionHelper(indices, marginals);

  const int64_t k = marginals.size();
  const int64_t dim = rSizes.size();

  std::vector<NDArray<int64_t>> m;
  m.reserve(k);
  std::vector<std::vector<int64_t>> idx;
  idx.reserve(k);
  std::vector<int64_t> s;
  s.reserve(dim);

  if (indices.size() != marginals.size())
    throw std::runtime_error("no. of marginals not equal to no. of indices");

  // assemble dimensions (row major) for seed
  for (int64_t i = dim-1; i >= 0; --i)
    s.push_back(rSizes[(size_t)i]);

  // insert indices and marginals in reverse order (R being column-major)
  for (int64_t i = k-1; i >= 0; --i)
  {
    const IntegerVector& iv = indices[i];
    const IntegerVector& nv = marginals[i];
    idx.push_back(std::vector<int64_t>(iv.size()));
    // also need to reverse dimension indices
    for (size_t j = 0; j < iv.size(); ++j)
      idx.back()[j] = dim - iv[j];
    // convert to NDArray
    m.push_back(std::move(convertRArray<int64_t, IntegerVector>(nv)));
  }

  // Storage for result
  List result;
  // Do QIS (could provide another ctor that takes preallocated memory for result)
  QIS qis(idx, m, skips);

  // How painful can it be to initialise a multidimensional array?
  int64_t size = std::accumulate(rSizes.begin(), rSizes.end(), 1ll, std::multiplies<int64_t>());
  IntegerVector r(size);
  NumericVector e(size);
  r.attr("dim") = rSizes;
  e.attr("dim") = rSizes;
  // Copy result data into R array
  const NDArray<int64_t>& tmp = qis.solve();
  std::copy(tmp.rawData(), tmp.rawData() + tmp.storageSize(), r.begin());
  const NDArray<double>& tmpe = qis.expectation();
  std::copy(tmpe.rawData(), tmpe.rawData() + tmpe.storageSize(), e.begin());
  result["conv"] = qis.conv();
  result["result"] = r;
  result["expectation"] = e;
  result["pop"] = qis.population();
  result["chiSq"] = qis.chiSq();
  result["pValue"] = qis.pValue();
  result["degeneracy"] = qis.degeneracy();

  return result;
}

//' QIS-IPF
//'
//' C++ QIS-IPF implementation
//' @param seed an n-dimensional array of seed values
//' @param indices
//' @param marginals a List of n integer vectors containing marginal data. The sum of elements in each vector must be identical
//' @return an object containing: ...
//' @export
// [[Rcpp::export]]
List qisi(NumericVector seed, List indices, List marginals, int skips = 0)
{
  if (indices.size() != marginals.size())
  {
    throw std::runtime_error("index and marginal lists are different lengths");
  }

  const int64_t k = marginals.size();

  Dimension rSizes = seed.attr("dim");
  const int64_t dim = rSizes.size();

  std::vector<NDArray<int64_t>> m;
  m.reserve(k);
  std::vector<std::vector<int64_t>> idx;
  idx.reserve(k);
  std::vector<int64_t> s;
  s.reserve(dim);

  if (indices.size() != marginals.size())
    throw std::runtime_error("no. of marginals not equal to no. of indices");

  // assemble dimensions (row major) for seed
  for (int64_t i = dim-1; i >= 0; --i)
    s.push_back(rSizes[(size_t)i]);

    // insert indices and marginals in reverse order (R being column-major)
  for (int64_t i = k-1; i >= 0; --i)
  {
    const IntegerVector& iv = indices[i];
    const IntegerVector& mv = marginals[i];
    idx.push_back(std::vector<int64_t>(iv.size()));
    // also need to reverse dimension indices
    for (size_t j = 0; j < iv.size(); ++j)
      idx.back()[j] = dim - iv[j];
    // convert to NDArray
    m.push_back(std::move(convertRArray<int64_t, IntegerVector>(mv)));
  }

  IntegerVector r(rSizes);
  NumericVector e(rSizes);
  //e.attr("dim") = rSizes;

  List result;

  // Read-only shallow copy of seed
  const NDArray<double> seedwrapper(s, (double*)&seed[0]);
  // Do IPF
  QISI qisipf(idx, m, skips);
  // Copy result data into R array
  const NDArray<int64_t>& tmp = qisipf.solve(seedwrapper);
  // Copy result data into R array
  const NDArray<double>& tmpe = qisipf.expectation();
  std::copy(tmpe.rawData(), tmpe.rawData() + tmpe.storageSize(), e.begin());
  result["expectation"] = e;

  std::copy(tmp.rawData(), tmp.rawData() + tmp.storageSize(), r.begin());
  result["conv"] = qisipf.conv();
  result["result"] = r;
  result["pop"] = qisipf.population();
  result["chiSq"] = qisipf.chiSq();

  return result;
}


// ' Constraine a pre-generated population in 2 dimensions given a constraint matrix.
// '
// ' Using an iterative algorithm, this function
// ' adjusts a 2-dimensional population table, reassigning populations in disallowed states to allowed ones, preserving the two marginal distributions
// ' states where elements sum to the input marginals.
// ' Users need to ensure that the supplied constraint matrix permits a valid population to be computed - this is not always obvious from the input data.
// ' @param population an integer matrix containing the population.
// ' @param permittedStates a matrix of booleans containing allowed states. The matrix dimensions must be the length of each marginal
// ' @return an object containing: the adjusted population matrix and a convergence flag.
// ' @examples
// ' r = c(0, 3, 17, 124, 167, 79, 46, 22)
// ' # rooms (1,2,3...9+)
// ' b = c(0, 15, 165, 238, 33, 7) # bedrooms {0, 1,2...5+}
// ' p = matrix(rep(TRUE,length(r)*length(b)), nrow=length(r)) # all states permitted
// ' # now disallow bedrooms>rooms
// ' for (i in 1:length(r)) {
// '   for (j in 1:length(b)) {
// '     if (j > i + 1)
// '       p[i,j] = FALSE;
// '   }
// ' }
// ' res = humanleague::synthPop(list(r,b)) # unconstrained synthesis
// ' res = humanleague::constrain(res$x.hat, p)
// ' @export
// // [[Rcpp::export]]
// List constrain(IntegerMatrix population, LogicalMatrix permittedStates)
// {
//   size_t d[2] = { (size_t)population.rows(), (size_t)population.cols() };
//
//   if (permittedStates.rows() != d[0] || permittedStates.cols() != d[1])
//     throw std::runtime_error("CQIWS population / permittedStates matrix size mismatch");
//
//   NDArray<2,bool> permitted(d);
//   NDArray<2,uint32_t> pop(d);
//
//   size_t idx[2];
//
//   for (idx[0] = 0; idx[0] < d[0]; ++idx[0])
//   {
//     for (idx[1] = 0; idx[1] < d[1]; ++idx[1])
//     {
//       pop[idx] = population(idx[0],idx[1]);
//       permitted[idx] = permittedStates(idx[0],idx[1]);
//     }
//   }
//   //
//   List result;
//   doConstrain(result, pop, permitted);
//
//   return result;
// }

//' Generate integer frequencies from discrete probabilities and an overall population.
//'
//' This function will generate the closest integer vector to the probabilities scaled to the population.
//' @param pIn a numeric vector of state occupation probabilities. Must sum to unity (to within double precision epsilon)
//' @param pop the total population
//' @return an integer vector of frequencies that sums to pop.
//' @examples
//' prob2IntFreq(c(0.1,0.2,0.3,0.4), 11)
//' @export
// [[Rcpp::export]]
List prob2IntFreq(NumericVector pIn, int pop)
{
  double var;
  const std::vector<double>& p = as<std::vector<double>>(pIn);

  if (pop < 1)
  {
    throw std::runtime_error("population must be strictly positive");
  }

  if (fabs(std::accumulate(p.begin(), p.end(), -1.0)) > 1000*std::numeric_limits<double>::epsilon())
  {
    throw std::runtime_error("probabilities do not sum to unity");
  }
  std::vector<int> f = integeriseMarginalDistribution(p, pop, var);

  List result;
  result["freq"] = f;
  result["var"] = var;

  return result;
}

//' Generate Sobol' quasirandom sequence
//'
//' @param dim dimensions
//' @param n number of variates to sample
//' @param skip number of variates to skip (actual number skipped will be largest power of 2 less than k)
//' @return a n-by-d matrix of uniform probabilities in (0,1).
//' @examples
//' sobolSequence(2, 1000, 1000) # will skip 512 numbers!
//' @export
// [[Rcpp::export]]
NumericMatrix sobolSequence(int dim, int n, int skip = 0)
{
  static const double scale = 0.5 / (1ull<<31);

  NumericMatrix m(n, dim);

  Sobol s(dim, skip);

  for (int j = 0; j <n ; ++j)
    for (int i = 0; i < dim; ++i)
      m(j,i) = s() * scale;

  return m;
}


//' Generate correlated 2D Sobol' quasirandom sequence
//'
//' @param rho correlation
//' @param n number of variates to sample
//' @param skip number of variates to skip (actual number skipped will be largest power of 2 less than k)
//' @return a n-by-2 matrix of uniform correlated probabilities in (0,1).
//' @examples
//' correlatedSobol2Sequence(0.2, 1000)
//' @export
// [[Rcpp::export]]
NumericMatrix correlatedSobol2Sequence(double rho, int n, int skip = 0)
{
  static const double scale = 0.5 / (1ull<<31);

  NumericMatrix m(n, 2);

  Sobol s(2, skip);

  Cholesky cholesky(rho);
  for (int j = 0; j <n ; ++j)
  {
    const std::pair<uint32_t, uint32_t>& buf = cholesky(s.buf());
    m(j,0) = buf.first * scale;
    m(j,1) = buf.second * scale;
  }

  return m;
}

//' Convert multidimensional array of counts per state into table form. Each row in the table corresponds to one individual
//'
//' This function
//' @param stateOccupancies an arbitrary-dimension array of (integer) state occupation counts.
//' @param categoryNames a string vector of unique column names.
//' @return a DataFrame with columns corresponding to category values and rows corresponding to individuals.
//' @export
// [[Rcpp::export]]
DataFrame flatten(IntegerVector stateOccupancies, StringVector categoryNames)
{
  //m.push_back(std::move(convertRArray<int64_t, IntegerVector>(nv)));
  const NDArray<int64_t>& a = convertRArray<int64_t, IntegerVector>(stateOccupancies);
  int64_t pop = sum(a);

  // for R indices start at 1
  const std::vector<std::vector<int>>& list = listify(pop, a, 1);

  // DataFrame interface is poor and appears buggy. Best approach seems to insert columns in List then assign to DataFrame at end
  List proxyDf;
  std::string s("C");
  for (size_t i = 0; i < a.dim(); ++i)
  {
    proxyDf[as<std::string>(categoryNames[i])] = list[i];
  }

  return DataFrame(proxyDf);
}

//' Entry point to enable running unit tests within R (e.g. in testthat)
//'
//' @return a List containing, number of tests run, number of failures, and any error messages.
//' @examples
//' unitTest()
//' @export
// [[Rcpp::export]]
List unitTest()
{
  const unittest::Logger& log = unittest::run();

  List result;
  result["nTests"] = log.testsRun;
  result["nFails"] = log.testsFailed;
  result["errors"] = log.errors;

  return result;
}
