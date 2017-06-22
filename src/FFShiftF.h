#include <stdio.h>

#ifndef FF_FSHIFT_H	//SMR
#define FF_FSHIFT_H	//SMR

#include "EnsemblePreprocessor.h" //For "MIE_INT_ONLY" preprocessor.
#include "FFConst.h" //constants related to particles.
#include "BasicTypes.h" //for uint
#include "NumLib.h" //For Cb, Sq
#include "FFParticle.h"

//////////////////////////////////////////////////////////////////////
////////////////////////// LJ Shift Style ////////////////////////////
//////////////////////////////////////////////////////////////////////
// Virial and LJ potential calculation:
// U(rij) = cn * eps_ij * ( (sig_ij/rij)^n - (sig_ij/rij)^6) + shiftConst
// shiftConst = cn * eps_ij * ( (sig_ij/rcut)^n - (sig_ij/rcut)^6)
// cn = n/(n-6) * ((n/6)^(6/(n-6)))
//
// Vir(r) = cn * eps_ij * 6 * ((n/6) * repulse - attract)/rij^2
// U_lrc = 0
// Vir_lrc = 0
//
// Eelect = qi * qj * (1/r - 1/rcut)
// Welect = qi * qj * 1/rij^3

struct FF_FSHIFT : public FFParticle
{
public:

  virtual double CalcEn(const double distSq,
                        const uint kind1, const uint kind2) const;
  virtual double CalcVir(const double distSq,
                         const uint kind1, const uint kind2) const;
  virtual void CalcAdd_1_4(double& en, const double distSq,
                           const uint kind1, const uint kind2) const;

  // coulomb interaction functions
  virtual double CalcCoulombEn(const double distSq,
                               const double qi_qj_Fact) const;
  virtual double CalcCoulombVir(const double distSq,
                                const double qi_qj) const;
  virtual void CalcCoulombAdd_1_4(double& en, const double distSq,
                                  const double qi_qj_Fact) const;

  //!Returns Ezero, no energy correction
  virtual double EnergyLRC(const uint kind1, const uint kind2) const
  {
    return 0.0;
  }
  //!!Returns Ezero, no virial correction
  virtual double VirialLRC(const uint kind1, const uint kind2) const
  {
    return 0.0;
  }
};


inline void FF_FSHIFT::CalcAdd_1_4(double& en, const double distSq,
                                  const uint kind1, const uint kind2) const
{
  uint index = FlatIndex(kind1, kind2);
  double rRat2 = sigmaSq_1_4[index]/distSq;
  double rRat4 = rRat2 * rRat2;
  double attract = rRat4 * rRat2;
#ifdef MIE_INT_ONLY
  uint n_ij = n_1_4[index];
  double repulse = num::POW(rRat2, rRat4, attract, n_ij);
#else
  double n_ij = n_1_4[index];
  double repulse = pow(sqrt(rRat2), n_ij);
#endif

  //en += (epsilon_cn_1_4[index] * (repulse-attract) - shiftConst_1_4[index]);
  en +=   (epsilon_cn_1_4[index] * (repulse-attract) - shiftConst_1_4[index] + ( 1.0/(3.0*sqrt(rRat2)) -1 ) *fshiftConst_1_4[index]);	//SMR
}

inline void FF_FSHIFT::CalcCoulombAdd_1_4(double& en, const double distSq,
    const double qi_qj_Fact) const
{
  if(ewald)
  {
     double dist = sqrt(distSq);
     double erfc = alpha * dist;
     en += scaling_14 * qi_qj_Fact * (1 - erf(erfc))/ dist;
  }
  else
  {
     double dist = sqrt(distSq);
     en += scaling_14 * qi_qj_Fact * (1.0/dist - 1.0/rCut);
  }
}


//mie potential
inline double FF_FSHIFT::CalcEn(const double distSq,
                               const uint kind1, const uint kind2) const
{
  uint index = FlatIndex(kind1, kind2);
  double rRat2 = sigmaSq[index]/distSq;
  double rRat4 = rRat2 * rRat2;
  double attract = rRat4 * rRat2;
#ifdef MIE_INT_ONLY
  uint n_ij = n[index];
  double repulse = num::POW(rRat2, rRat4, attract, n_ij);
#else
  double n_ij = n[index];
  double repulse = pow(sqrt(rRat2), n_ij);
#endif

  //return (epsilon_cn[index] * (repulse-attract) - shiftConst[index]);
  return (epsilon_cn[index] * (repulse-attract) - shiftConst[index]) + ( 1.0/(3.0*sqrt(rRat2))-1 ) *fshiftConst[index];	//SMR
}

inline double FF_FSHIFT::CalcCoulombEn(const double distSq,
                                      const double qi_qj_Fact) const
{
  if(distSq <= rCutLowSq)
    return num::BIGNUM;

  if(ewald)
  {
     double dist = sqrt(distSq);
     double erfc = alpha * dist;
     return  qi_qj_Fact * (1 - erf(erfc))/ dist;
  }
  else
  {
     double dist = sqrt(distSq);
     return  qi_qj_Fact * (1.0/dist - 1.0/rCut);
  }
}

//mie potential
inline double FF_FSHIFT::CalcVir(const double distSq,
                                const uint kind1, const uint kind2) const
{
  uint index = FlatIndex(kind1, kind2);
  double rNeg2 = 1.0/distSq;
  double rRat2 = rNeg2 * sigmaSq[index];
  double rRat4 = rRat2 * rRat2;
  double attract = rRat4 * rRat2;
#ifdef MIE_INT_ONLY
  uint n_ij = n[index];
  double repulse = num::POW(rRat2, rRat4, attract, n_ij);
#else
  double n_ij = n[index];
  double repulse = pow(sqrt(rRat2), n_ij);
#endif

  //Virial is the derivative of the pressure... mu
  //return epsilon_cn_6[index] * (nOver6[index]*repulse-attract)*rNeg2;
  return epsilon_cn_6[index] * (nOver6[index]*repulse-attract)*rNeg2 - fshiftConst[index]/sqrt(9.0*distSq*sigmaSq[index]);  //SMR
}

inline double FF_FSHIFT::CalcCoulombVir(const double distSq,
                                       const double qi_qj) const
{
  if(ewald)
  {
     double dist = sqrt(distSq);
     double constValue = 2.0 * alpha / sqrt(M_PI);
     double expConstValue = exp(-1.0 * alpha * alpha * distSq);
     double temp = 1.0 - erf(alpha * dist);
     return  qi_qj * (temp / dist + constValue * expConstValue) / distSq;
  }
  else
  {
     double dist = sqrt(distSq);
     return qi_qj/(distSq * dist);
  }
}


#endif /*FF_FSHIFT_H*/
