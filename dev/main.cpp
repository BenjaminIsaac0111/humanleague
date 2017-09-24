
#include "src/IPF.h"
#include "src/QSIPF.h"
#include "src/Sobol.h"

// deprecated
#include "src/NDArrayUtilsOld.h"
#include "src/IndexOld.h"

// work-in-progress
#include "src/NDArray.h"
#include "src/Index.h"
#include "src/NDArrayUtils.h"

#include <map>
#include <iostream>

// TODO integer marginals

void do2d()
{
  std::vector<std::vector<int64_t>> m = {std::vector<int64_t>{52, 48}, 
                                        std::vector<int64_t>{10, 77, 13}};
                                    
  std::vector<int64_t> size{ (int64_t)m[0].size(), (int64_t)m[1].size() };                                        

  NDArray<double> s(size);
  s.assign(1.0);
  //Index index(s.sizes());
  //s[index] = 0.5;

  QSIPF qsipf(s, m);

  auto e = qsipf.errors();
  print(e[0]);
  print(e[1]);
  std::cout << qsipf.conv() << ":" << qsipf.iters() << ":" << qsipf.chiSq() << std::endl;
  print(qsipf.result().rawData(), qsipf.result().storageSize(), m[1].size());
  print(qsipf.sample().rawData(), qsipf.sample().storageSize(), qsipf.sample().sizes()[1]);
  print(reduce<int64_t>(qsipf.sample(), 0));
  print(reduce<int64_t>(qsipf.sample(), 1));
}

void do2dIPF()
{
  std::vector<std::vector<double>> m = {std::vector<double>{12, 40, 48}, 
                                        std::vector<double>{87, 13}};
                                    
  std::vector<int64_t> size{ (int64_t)m[0].size(), (int64_t)m[1].size() };                                        

  NDArray<double> s(size);
  s.assign(1.0);
  //Index index(s.sizes());
  //s[index] = 0.5;

  IPF ipf(s, m);

  auto e = ipf.errors();
  print(e[0]);
  print(e[1]);
  std::cout << ipf.conv() << ":" << ipf.iters() << std::endl;
  print(ipf.result().rawData(), ipf.result().storageSize(), m[1].size());
  print(reduce(ipf.result(), 0));
  print(reduce(ipf.result(), 1));
}


void do3d()
{
  std::vector<std::vector<int64_t>> m = {std::vector<int64_t>{52, 40, 4, 4}, 
                                        std::vector<int64_t>{87, 10, 3},
                                        std::vector<int64_t>{55, 15, 6,12, 12}};
  // std::vector<std::vector<double>> m = {std::vector<int64_t>{32, 32, 32, 32}, 
  //                                       std::vector<int64_t>{126, 1, 1},
  //                                       std::vector<int64_t>{64, 64}};

  std::vector<int64_t> size{ (int64_t)m[0].size(), (int64_t)m[1].size(), (int64_t)m[2].size() };                                        

  NDArray<double> s(size);
  s.assign(1.0);
  Index i(s.sizes());
  s[i] = 1.0;

  QSIPF qsipf(s, m);

  auto e = qsipf.errors();
  print(e[0]);
  print(e[1]);
  print(e[2]);
  // conv/iters/errors needs a rethink
  std::cout << qsipf.conv() << ":" << qsipf.iters() << ":" << qsipf.chiSq() << std::endl;
  //print(qsipf.result().rawData(), qsipf.result().storageSize(), m[1].size());

  //print(pop.rawData(), pop.storageSize(), pop.sizes()[1]);
  print(qsipf.sample().rawData(), qsipf.sample().storageSize(), qsipf.sample().sizes()[1]);
  print(reduce<int64_t>(qsipf.sample(), 0));
  print(reduce<int64_t>(qsipf.sample(), 1));
  print(reduce<int64_t>(qsipf.sample(), 2));
}

void do3dIPF()
{
  std::vector<std::vector<double>> m = {std::vector<double>{52, 48}, 
                                        std::vector<double>{10, 77, 13},
                                        std::vector<double>{20, 27, 30, 23}};
  
  std::vector<int64_t> size{ (int64_t)m[0].size(), (int64_t)m[1].size(), (int64_t)m[2].size() };                                        

  NDArray<double> s(size);
  s.assign(1.0);
  //Index index(s.sizes());
  //s[index] = 0.5;

  IPF ipf(s, m);

  auto e = ipf.errors();
  print(e[0]);
  print(e[1]);
  print(e[2]);
  std::cout << ipf.conv() << ":" << ipf.iters() << std::endl;
  print(ipf.result().rawData(), ipf.result().storageSize(), m[1].size());
  print(reduce(ipf.result(), 0));
  print(reduce(ipf.result(), 1));
  print(reduce(ipf.result(), 2));
}



void do4d()
{
  std::vector<std::vector<int64_t>> m = {std::vector<int64_t>{52, 48}, 
                                        std::vector<int64_t>{87, 13},
                                        std::vector<int64_t>{67, 33},
                                        std::vector<int64_t>{55, 45}};

  std::vector<int64_t> size{ (int64_t)m[0].size(), (int64_t)m[1].size(), (int64_t)m[2].size(), (int64_t)m[3].size() };                                        

  NDArray<double> s(size);
  s.assign(1.0);
  //Index index(s.sizes());
  //s[index] = 0.7;

  QSIPF qsipf(s, m);

  auto e = qsipf.errors();
  print(e[0]);
  print(e[1]);
  print(e[2]);
  print(e[3]);
  std::cout << qsipf.conv() << ":" << qsipf.iters() << ":" << qsipf.chiSq() << std::endl;
  print(qsipf.sample().rawData(), qsipf.sample().storageSize(), m[1].size());
  print(reduce<int64_t>(qsipf.sample(), 0));
  print(reduce<int64_t>(qsipf.sample(), 1));
  print(reduce<int64_t>(qsipf.sample(), 2));
  print(reduce<int64_t>(qsipf.sample(), 3));
}

class MIPF
{
public:
  typedef std::vector<int64_t> index_t;
  typedef std::vector<index_t> index_list_t;
  typedef NDArray<double> marginal_t;
  typedef std::vector<marginal_t> marginal_list_t;

  MIPF(const index_list_t& i, const marginal_list_t& m)
  {
    // i and m should be same size and >
    if (i.size() != m.size() || i.size() < 2)
      throw std::runtime_error("index and marginal lists differ in size or too small");

    // count all unique values in i...
    std::map<int64_t, int64_t> index_dim;   
    for (size_t k = 0; k < i.size(); ++k)
    {
      if (i[k].size() != m[k].dim())
        throw std::runtime_error("index/marginal dimension mismatch " + std::to_string(i[k].size()) + " vs " + std::to_string(m[k].dim()));
      //std::cout << "index " << k << std::endl;
      for (size_t j = 0; j < i[k].size(); ++j)
      {
        int64_t dim = i[k][j];
        int64_t size = m[k].size(j);
        // check if already entered that size is same
        auto posit = index_dim.find(dim);
        if (posit == index_dim.end())
          index_dim.insert(std::make_pair(dim, size));
        else if (posit->second != size)
          throw std::runtime_error("mismatch at index " + std::to_string(k) + 
            ": dimension " + std::to_string(dim) + " size " + std::to_string(posit->second) + " redefined to " + std::to_string(size));
        //std::cout << "  " << dim << ":" << size << std::endl;
      }
    }

    // check all dims defined
    std::vector<int64_t> sizes;
    sizes.reserve(i.size());
    
//    std::transform(index_dim.begin(), index_dim.end(), std::back_inserter(sizes),
//      [](const std::map<int64_t,int64_t>::value_type& pair) { return pair.first; });

    // we should expect that the index_dim map contains keys from 0 to index_dim.size()-1. if not throw
    for (size_t k = 0; k < index_dim.size(); ++k)
    {
      auto it = index_dim.find(k);
      if (it == index_dim.end())
        throw std::runtime_error("dimension " + std::to_string(k) + " size not defined");
      sizes.push_back(it->second);
    }

    for (size_t k = 0; k < sizes.size(); ++k)
    {  
      std::cout << k << ":" << sizes[k] << std::endl;
    }
  }
};

void doMd()
{
  std::vector<NDArray<double>> m;
  std::vector<std::vector<int64_t>> i;
  i.push_back(std::vector<int64_t>{0,1});
  i.push_back(std::vector<int64_t>{2,1});
  m.push_back(std::move(NDArray<double>(std::vector<int64_t>{2, 3})));
  m.push_back(std::move(NDArray<double>(std::vector<int64_t>{5, 3})));
  MIPF(i, m);
  
}

int main()
{
  try
  {
    do2d();
    do3d();
    do4d();

    std::vector<int64_t> s{3,2,5};
    NDArray<double> a(s);
    a.assign(1.0);
    for (Index index(a.sizes(), {0,1}); !index.end(); ++index)
    {
      ++a[index];
    }
    for (Index index(a.sizes(), {1,0}); !index.end(); ++index)
    {
      ++a[index];
    }
    for (Index index(a.sizes(), {2,2}); !index.end(); ++index)
    {
      ++a[index];
    }
    print(a.rawData(), a.storageSize(), s[2]);

    print(reduce(a, 0));
    print(reduce(a, 1));
    print(reduce(a, 2));

    for (size_t d = 0; d < a.dim(); ++d)
    {
      for (int64_t i = 0; i < a.sizes()[d]; ++i)
      {
        NDArray<double> a00 = slice(a, {d,i});
        print(a00.rawData(), a00.storageSize());//, a00.sizes()[1]);
      }
    }

    do2dIPF();
    do3dIPF();

    {
      NDArray<double> r = reduce(a, std::vector<int64_t>{0,1});


      std::cout << r.dim() << std::endl;
      print(r.sizes());
      print(r.rawData(), r.storageSize(), r.sizes()[1]);
    }
    {
      NDArray<double> r = reduce(a, std::vector<int64_t>{1,2});

      std::cout << r.dim() << std::endl;
      print(r.sizes());
      print(r.rawData(), r.storageSize(), r.sizes()[1]);
    }
    {
      NDArray<double> r = reduce(a, std::vector<int64_t>{2, 0});

      std::cout << r.dim() << std::endl;
      print(r.sizes());
      print(r.rawData(), r.storageSize(), r.sizes()[1]);
    }

    doMd();
  }
  catch(const std::exception& e)
  {
    std::cout << e.what() << std::endl;
  }
  catch(...)
  {
    std::cout << "unknown exception" << std::endl;
  }  
}