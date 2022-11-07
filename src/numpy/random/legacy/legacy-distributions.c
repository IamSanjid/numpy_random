/*
 * This file contains generation code for distribution that have been modified
 * since Generator was introduced. These are preserved using identical code
 * to what was in NumPy 1.16 so that the stream of values generated by
 * RandomState is not changed when there are changes that affect Generator.
 *
 * These functions should not be changed except if they contain code that
 * cannot be compiled. They should not be changed for bug fixes, performance
 * improvements that can change the values produced, or enhancements to precision.
 */
#include "legacy-distributions.h"


static NPY_INLINE double legacy_double(aug_bitgen_t *aug_state) {
  return aug_state->bit_generator->next_double(aug_state->bit_generator->state);
}

double legacy_gauss(aug_bitgen_t *aug_state) {
  if (aug_state->has_gauss) {
    const double temp = aug_state->gauss;
    aug_state->has_gauss = false;
    aug_state->gauss = 0.0;
    return temp;
  } else {
    double f, x1, x2, r2;

    do {
      x1 = 2.0 * legacy_double(aug_state) - 1.0;
      x2 = 2.0 * legacy_double(aug_state) - 1.0;
      r2 = x1 * x1 + x2 * x2;
    } while (r2 >= 1.0 || r2 == 0.0);

    /* Polar method, a more efficient version of the Box-Muller approach. */
    f = sqrt(-2.0 * log(r2) / r2);
    /* Keep for next call */
    aug_state->gauss = f * x1;
    aug_state->has_gauss = true;
    return f * x2;
  }
}

double legacy_standard_exponential(aug_bitgen_t *aug_state) {
  /* We use -log(1-U) since U is [0, 1) */
  return -log(1.0 - legacy_double(aug_state));
}

double legacy_standard_gamma(aug_bitgen_t *aug_state, double shape) {
  double b, c;
  double U, V, X, Y;

  if (shape == 1.0) {
    return legacy_standard_exponential(aug_state);
  }
  else if (shape == 0.0) {
    return 0.0;
  } else if (shape < 1.0) {
    for (;;) {
      U = legacy_double(aug_state);
      V = legacy_standard_exponential(aug_state);
      if (U <= 1.0 - shape) {
        X = pow(U, 1. / shape);
        if (X <= V) {
          return X;
        }
      } else {
        Y = -log((1 - U) / shape);
        X = pow(1.0 - shape + shape * Y, 1. / shape);
        if (X <= (V + Y)) {
          return X;
        }
      }
    }
  } else {
    b = shape - 1. / 3.;
    c = 1. / sqrt(9 * b);
    for (;;) {
      do {
        X = legacy_gauss(aug_state);
        V = 1.0 + c * X;
      } while (V <= 0.0);

      V = V * V * V;
      U = legacy_double(aug_state);
      if (U < 1.0 - 0.0331 * (X * X) * (X * X))
        return (b * V);
      if (log(U) < 0.5 * X * X + b * (1. - V + log(V)))
        return (b * V);
    }
  }
}

double legacy_gamma(aug_bitgen_t *aug_state, double shape, double scale) {
  return scale * legacy_standard_gamma(aug_state, shape);
}

double legacy_pareto(aug_bitgen_t *aug_state, double a) {
  return exp(legacy_standard_exponential(aug_state) / a) - 1;
}

double legacy_weibull(aug_bitgen_t *aug_state, double a) {
  if (a == 0.0) {
    return 0.0;
  }
  return pow(legacy_standard_exponential(aug_state), 1. / a);
}

double legacy_power(aug_bitgen_t *aug_state, double a) {
  return pow(1 - exp(-legacy_standard_exponential(aug_state)), 1. / a);
}

double legacy_chisquare(aug_bitgen_t *aug_state, double df) {
  return 2.0 * legacy_standard_gamma(aug_state, df / 2.0);
}

double legacy_rayleigh(bitgen_t *bitgen_state, double mode) {
  return mode * sqrt(-2.0 * npy_log1p(-next_double(bitgen_state)));
}

double legacy_noncentral_chisquare(aug_bitgen_t *aug_state, double df,
                                   double nonc) {
  double out;
  if (nonc == 0) {
    return legacy_chisquare(aug_state, df);
  }
  if (1 < df) {
    const double Chi2 = legacy_chisquare(aug_state, df - 1);
    const double n = legacy_gauss(aug_state) + sqrt(nonc);
    return Chi2 + n * n;
  } else {
    const long i = random_poisson(aug_state->bit_generator, nonc / 2.0);
    out = legacy_chisquare(aug_state, df + 2 * i);
    /* Insert nan guard here to avoid changing the stream */
    if (npy_isnan(nonc)){
      return NPY_NAN;
    } else {
    return out;
    }
  }
}

double legacy_noncentral_f(aug_bitgen_t *aug_state, double dfnum, double dfden,
                           double nonc) {
  double t = legacy_noncentral_chisquare(aug_state, dfnum, nonc) * dfden;
  return t / (legacy_chisquare(aug_state, dfden) * dfnum);
}

double legacy_wald(aug_bitgen_t *aug_state, double mean, double scale) {
  double U, X, Y;
  double mu_2l;

  mu_2l = mean / (2 * scale);
  Y = legacy_gauss(aug_state);
  Y = mean * Y * Y;
  X = mean + mu_2l * (Y - sqrt(4 * scale * Y + Y * Y));
  U = legacy_double(aug_state);
  if (U <= mean / (mean + X)) {
    return X;
  } else {
    return mean * mean / X;
  }
}

double legacy_normal(aug_bitgen_t *aug_state, double loc, double scale) {
  return loc + scale * legacy_gauss(aug_state);
}

double legacy_lognormal(aug_bitgen_t *aug_state, double mean, double sigma) {
  return exp(legacy_normal(aug_state, mean, sigma));
}

double legacy_standard_t(aug_bitgen_t *aug_state, double df) {
  double num, denom;

  num = legacy_gauss(aug_state);
  denom = legacy_standard_gamma(aug_state, df / 2);
  return sqrt(df / 2) * num / sqrt(denom);
}

int64_t legacy_negative_binomial(aug_bitgen_t *aug_state, double n, double p) {
  double Y = legacy_gamma(aug_state, n, (1 - p) / p);
  return (int64_t)random_poisson(aug_state->bit_generator, Y);
}

double legacy_standard_cauchy(aug_bitgen_t *aug_state) {
  return legacy_gauss(aug_state) / legacy_gauss(aug_state);
}

double legacy_beta(aug_bitgen_t *aug_state, double a, double b) {
  double Ga, Gb;

  if ((a <= 1.0) && (b <= 1.0)) {
    double U, V, X, Y;
    /* Use Johnk's algorithm */

    while (1) {
      U = legacy_double(aug_state);
      V = legacy_double(aug_state);
      X = pow(U, 1.0 / a);
      Y = pow(V, 1.0 / b);

      if ((X + Y) <= 1.0) {
        if (X + Y > 0) {
          return X / (X + Y);
        } else {
          double logX = log(U) / a;
          double logY = log(V) / b;
          double logM = logX > logY ? logX : logY;
          logX -= logM;
          logY -= logM;

          return exp(logX - log(exp(logX) + exp(logY)));
        }
      }
    }
  } else {
    Ga = legacy_standard_gamma(aug_state, a);
    Gb = legacy_standard_gamma(aug_state, b);
    return Ga / (Ga + Gb);
  }
}

double legacy_f(aug_bitgen_t *aug_state, double dfnum, double dfden) {
  return ((legacy_chisquare(aug_state, dfnum) * dfden) /
          (legacy_chisquare(aug_state, dfden) * dfnum));
}

double legacy_exponential(aug_bitgen_t *aug_state, double scale) {
  return scale * legacy_standard_exponential(aug_state);
}


static RAND_INT_TYPE legacy_random_binomial_original(bitgen_t *bitgen_state,
                                                     double p,
                                                     RAND_INT_TYPE n,
                                                     binomial_t *binomial) {
  double q;

  if (p <= 0.5) {
    if (p * n <= 30.0) {
      return random_binomial_inversion(bitgen_state, n, p, binomial);
    } else {
      return random_binomial_btpe(bitgen_state, n, p, binomial);
    }
  } else {
    q = 1.0 - p;
    if (q * n <= 30.0) {
      return n - random_binomial_inversion(bitgen_state, n, q, binomial);
    } else {
      return n - random_binomial_btpe(bitgen_state, n, q, binomial);
    }
  }
}


int64_t legacy_random_binomial(bitgen_t *bitgen_state, double p,
                               int64_t n, binomial_t *binomial) {
  return (int64_t) legacy_random_binomial_original(bitgen_state, p,
                                                   (RAND_INT_TYPE) n,
                                                   binomial);
}


static RAND_INT_TYPE random_hypergeometric_hyp(bitgen_t *bitgen_state,
                                               RAND_INT_TYPE good,
                                               RAND_INT_TYPE bad,
                                               RAND_INT_TYPE sample) {
  RAND_INT_TYPE d1, k, z;
  double d2, u, y;

  d1 = bad + good - sample;
  d2 = (double)MIN(bad, good);

  y = d2;
  k = sample;
  while (y > 0.0) {
    u = next_double(bitgen_state);
    y -= (RAND_INT_TYPE)floor(u + y / (d1 + k));
    k--;
    if (k == 0)
      break;
  }
  z = (RAND_INT_TYPE)(d2 - y);
  if (good > bad)
    z = sample - z;
  return z;
}

/* D1 = 2*sqrt(2/e) */
/* D2 = 3 - 2*sqrt(3/e) */
#define D1 1.7155277699214135
#define D2 0.8989161620588988
static RAND_INT_TYPE random_hypergeometric_hrua(bitgen_t *bitgen_state,
                                                RAND_INT_TYPE good,
                                                RAND_INT_TYPE bad,
                                                RAND_INT_TYPE sample) {
  RAND_INT_TYPE mingoodbad, maxgoodbad, popsize, m, d9;
  double d4, d5, d6, d7, d8, d10, d11;
  RAND_INT_TYPE Z;
  double T, W, X, Y;

  mingoodbad = MIN(good, bad);
  popsize = good + bad;
  maxgoodbad = MAX(good, bad);
  m = MIN(sample, popsize - sample);
  d4 = ((double)mingoodbad) / popsize;
  d5 = 1.0 - d4;
  d6 = m * d4 + 0.5;
  d7 = sqrt((double)(popsize - m) * sample * d4 * d5 / (popsize - 1) + 0.5);
  d8 = D1 * d7 + D2;
  d9 = (RAND_INT_TYPE)floor((double)(m + 1) * (mingoodbad + 1) / (popsize + 2));
  d10 = (random_loggam(d9 + 1) + random_loggam(mingoodbad - d9 + 1) +
         random_loggam(m - d9 + 1) + random_loggam(maxgoodbad - m + d9 + 1));
  d11 = MIN(MIN(m, mingoodbad) + 1.0, floor(d6 + 16 * d7));
  /* 16 for 16-decimal-digit precision in D1 and D2 */

  while (1) {
    X = next_double(bitgen_state);
    Y = next_double(bitgen_state);
    W = d6 + d8 * (Y - 0.5) / X;

    /* fast rejection: */
    if ((W < 0.0) || (W >= d11))
      continue;

    Z = (RAND_INT_TYPE)floor(W);
    T = d10 - (random_loggam(Z + 1) + random_loggam(mingoodbad - Z + 1) +
               random_loggam(m - Z + 1) + random_loggam(maxgoodbad - m + Z + 1));

    /* fast acceptance: */
    if ((X * (4.0 - X) - 3.0) <= T)
      break;

    /* fast rejection: */
    if (X * (X - T) >= 1)
      continue;
    /* log(0.0) is ok here, since always accept */
    if (2.0 * log(X) <= T)
      break; /* acceptance */
  }

  /* this is a correction to HRUA* by Ivan Frohne in rv.py */
  if (good > bad)
    Z = m - Z;

  /* another fix from rv.py to allow sample to exceed popsize/2 */
  if (m < sample)
    Z = good - Z;

  return Z;
}
#undef D1
#undef D2

static RAND_INT_TYPE random_hypergeometric_original(bitgen_t *bitgen_state,
                                                    RAND_INT_TYPE good,
                                                    RAND_INT_TYPE bad,
                                                    RAND_INT_TYPE sample)
{
  if (sample > 10) {
    return random_hypergeometric_hrua(bitgen_state, good, bad, sample);
  } else if (sample > 0) {
    return random_hypergeometric_hyp(bitgen_state, good, bad, sample);
  } else {
    return 0;
  }
}


/*
 * This is a wrapper function that matches the expected template. In the legacy
 * generator, all int types are long, so this accepts int64 and then converts
 * them to longs. These values must be in bounds for long and this is checked
 * outside this function
 *
 * The remaining are included for the return type only
 */
int64_t legacy_random_hypergeometric(bitgen_t *bitgen_state, int64_t good,
                                     int64_t bad, int64_t sample) {
  return (int64_t)random_hypergeometric_original(bitgen_state,
                                                 (RAND_INT_TYPE)good,
                                                 (RAND_INT_TYPE)bad,
                                                 (RAND_INT_TYPE)sample);
}


int64_t legacy_random_poisson(bitgen_t *bitgen_state, double lam) {
  return (int64_t)random_poisson(bitgen_state, lam);
}

int64_t legacy_random_zipf(bitgen_t *bitgen_state, double a) {
  return (int64_t)random_zipf(bitgen_state, a);
}


static long legacy_geometric_inversion(bitgen_t *bitgen_state, double p) {
  return (long)ceil(npy_log1p(-next_double(bitgen_state)) / log(1 - p));
}

int64_t legacy_random_geometric(bitgen_t *bitgen_state, double p) {
  if (p >= 0.333333333333333333333333) {
    return (int64_t)random_geometric_search(bitgen_state, p);
  } else {
    return (int64_t)legacy_geometric_inversion(bitgen_state, p);
  }
}

void legacy_random_multinomial(bitgen_t *bitgen_state, RAND_INT_TYPE n,
                               RAND_INT_TYPE *mnix, double *pix, npy_intp d,
                               binomial_t *binomial) {
  random_multinomial(bitgen_state, n, mnix, pix, d, binomial);
}

double legacy_vonmises(bitgen_t *bitgen_state, double mu, double kappa) {
  double s;
  double U, V, W, Y, Z;
  double result, mod;
  int neg;
  if (npy_isnan(kappa)) {
    return NPY_NAN;
  }
  if (kappa < 1e-8) {
    return M_PI * (2 * next_double(bitgen_state) - 1);
  } else {
    /* with double precision rho is zero until 1.4e-8 */
    if (kappa < 1e-5) {
      /*
       * second order taylor expansion around kappa = 0
       * precise until relatively large kappas as second order is 0
       */
      s = (1. / kappa + kappa);
    } else {
        /* Path for 1e-5 <= kappa <= 1e6 */
        double r = 1 + sqrt(1 + 4 * kappa * kappa);
        double rho = (r - sqrt(2 * r)) / (2 * kappa);
        s = (1 + rho * rho) / (2 * rho);
    }

    while (1) {
      U = next_double(bitgen_state);
      Z = cos(M_PI * U);
      W = (1 + s * Z) / (s + Z);
      Y = kappa * (s - W);
      V = next_double(bitgen_state);
      /*
       * V==0.0 is ok here since Y >= 0 always leads
       * to accept, while Y < 0 always rejects
       */
      if ((Y * (2 - Y) - V >= 0) || (log(Y / V) + 1 - Y >= 0)) {
        break;
      }
    }

    U = next_double(bitgen_state);

    result = acos(W);
    if (U < 0.5) {
      result = -result;
    }
    result += mu;
    neg = (result < 0);
    mod = fabs(result);
    mod = (fmod(mod + M_PI, 2 * M_PI) - M_PI);
    if (neg) {
      mod *= -1;
    }

    return mod;
  }
}

int64_t legacy_logseries(bitgen_t *bitgen_state, double p) {
  double q, r, U, V;
  long result;

  r = log(1.0 - p);

  while (1) {
    V = next_double(bitgen_state);
    if (V >= p) {
      return 1;
    }
    U = next_double(bitgen_state);
    q = 1.0 - exp(r * U);
    if (V <= q * q) {
      result = (long)floor(1 + log(V) / log(q));
      if ((result < 1) || (V == 0.0)) {
        continue;
      } else {
        return (int64_t)result;
      }
    }
    if (V >= q) {
      return 1;
    }
    return 2;
  }
}