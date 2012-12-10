#ifndef FEEDBACK_HINCLUDED
#define FEEDBACK_HINCLUDED

#include "parameters.h"
#include "supernova.h"
#include "imf.h"
#include "starlifetime.h"
#define NFEEDBACKS 4

/*
 * Class to return feedback effects.
 */
class FBEffects {
 public:
   double dEnergy;		/* Energy produced per mass (ergs/gm); note that
				   this might need to be divided up
				   into different forms of energy. */
    double dMassLoss;		/* Mass lost in Solar Masses */
    double dMetals;		/* Fraction of the mass lost in
				   elements heavier than Helium */
    double dMIron;              /* Solar masses of iron ejected */
    double dMOxygen;            /* Solar masses of oxygen ejected */
 FBEffects() : dEnergy(0), dMassLoss(0), dMetals(0), dMIron(0), dMOxygen(0) { }
 FBEffects(double dEnergy, double dMassLoss, double dMetals, double dMIron, double dMOxygen) : 
    dEnergy(dEnergy), dMassLoss(dMassLoss), dMetals(dMetals), dMIron(dMIron), dMOxygen(dMOxygen) { }
     };

/*
 * Class to Characterize Star Formation event.  This is input
 * needed for all feedback effects.
 */
class SFEvent {
 public:
    double dMass;            /* mass in star formation event in solar masses */
    double dTimeForm;      /* time of star formation event in years */
    double dMetals;           /*  metallicity of stars in event */
    double dMFracOxygen;           /*  metallicity of stars in event */
    double dMFracIron;           /*  metallicity of stars in event */
 SFEvent() : dMass(0), dTimeForm(0), dMetals(0), dMFracIron(0), dMFracOxygen(0) { }
 SFEvent(double mass, double tform, double mets, double fefrac, double oxfrac) : 
    dMass(mass), dTimeForm(tform), dMetals(mets), dMFracIron(fefrac), dMFracOxygen(oxfrac) { }
    };

class Fdbk : public PUP::able {
 private:
    Fdbk& operator=(const Fdbk& fb);
    void CalcWindFeedback(SFEvent *sfEvent, double dTime, 
			  double dDelta, FBEffects *fbEffects);
    void CalcUVFeedback(double dTime, double dDelta, FBEffects *fbEffects);

    char achIMF[32];	        /* initial mass function */
    int iRandomSeed;		/* seed for stochastic quantized feedback */
    double dErgPerGmUnit;	/* system specific energy in ergs/gm */
    double dGmUnit;		/* system mass in grams */
    double dGmPerCcUnit;	/* system density in gm/cc */
    double dErgUnit;		/* system energy in ergs */
    Padova pdva;
 public:
    SN sn;
    double dDeltaStarForm;
    double dSecUnit;		/* system time in seconds */
    int bSNTurnOffCooling;      /* turn off cooling or not */
    int bSmallSNSmooth;	/* smooth SN energy only over blast radius */
    int bShortCoolShutoff;      /* use snowplow time */
    double dExtraCoolShutoff;      /* multiplicative factor for shutoff time */
    double dRadPreFactor;       /* McKee + Ostriker size constant in system units */
    double dTimePreFactor;      /* McKee + Ostriker time constant in system units */
    int nSmoothFeedback;	/* number of particles to smooth feedback over*/
    IMF *imf;

    void AddParams(PRM prm);
    void CheckParams(PRM prm, struct parameters &param);
    void DoFeedback(GravityParticle *p, double dTime, double dDeltaYr, 
		    FBEffects *fbTotals);
    double NSNIa (double dMassT1, double dMassT2);
    Fdbk() {
	imf = new Kroupa01(); /* a default placeholder */
	}

    PUPable_decl(Fdbk);
    Fdbk(const Fdbk& fb);
    Fdbk(CkMigrateMessage *m) : PUP::able(m) {}
    ~Fdbk() {
	delete imf;
	}
    inline void pup(PUP::er &p);
    };

//  "Deep copy" constructer is need because of imf pointer.
inline Fdbk::Fdbk(const Fdbk& fb) {
    strcpy(achIMF, fb.achIMF);
    dDeltaStarForm = fb.dDeltaStarForm;
    iRandomSeed = fb.iRandomSeed;
    dErgPerGmUnit = fb.dErgPerGmUnit;
    dGmUnit = fb.dGmUnit;
    dGmPerCcUnit = fb.dGmPerCcUnit;
    dErgUnit = fb.dErgUnit;
    dSecUnit = fb.dSecUnit;
    bSNTurnOffCooling = fb.bSNTurnOffCooling;
    bSmallSNSmooth = fb.bSmallSNSmooth;
    bShortCoolShutoff = fb.bShortCoolShutoff;
    dExtraCoolShutoff = fb.dExtraCoolShutoff;
    dRadPreFactor = fb.dRadPreFactor;
    dTimePreFactor = fb.dTimePreFactor;
    nSmoothFeedback = fb.nSmoothFeedback;
    sn = fb.sn;
    pdva = fb.pdva;
    imf = fb.imf->clone();
}

inline void Fdbk::pup(PUP::er &p) {
    p(achIMF, 32);
    p | dDeltaStarForm;
    p | iRandomSeed;
    p | dErgPerGmUnit;
    p | dGmUnit;
    p | dGmPerCcUnit;
    p | dErgUnit;
    p | dSecUnit;
    p | bSNTurnOffCooling;
    p | bSmallSNSmooth;
    p | bShortCoolShutoff;
    p | dExtraCoolShutoff;
    p | dRadPreFactor;
    p | dTimePreFactor;
    p | nSmoothFeedback;
    p | sn;
    p | pdva;
    p | imf;
    }

enum FBenum{
  FB_SNII=0,
  FB_SNIA,
  FB_WIND,
  FB_UV
};

#include "smoothparams.h"

/*
 * SmoothParams class for distributing stellar feedback (energy, mass + metals) 
 * to neighboring particles.
 */

class DistStellarFeedbackSmoothParams : public SmoothParams
{
    double dTime, H, a, gamma;
    Fdbk fb;
    virtual void fcnSmooth(GravityParticle *p, int nSmooth,
			   pqSmoothNode *nList);
    virtual int isSmoothActive(GravityParticle *p);
    virtual void initSmoothParticle(GravityParticle *p) {}
    virtual void initTreeParticle(GravityParticle *p);
    virtual void postTreeParticle(GravityParticle *p);
    virtual void initSmoothCache(GravityParticle *p);
    virtual void combSmoothCache(GravityParticle *p1,
				 ExternalSmoothParticle *p2);
    void DistFBMME(GravityParticle *p, int nSmooth, pqSmoothNode *nList);
 public:
    DistStellarFeedbackSmoothParams() {}
    DistStellarFeedbackSmoothParams(int _iType, int am, CSM csm, double _dTime,
				    double _gamma, Fdbk *feedback) : 
				    fb (*feedback) {
	iType = _iType;
	activeRung = am;
	bUseBallMax = 0;
	gamma = _gamma;
	dTime = _dTime;
	if(csm->bComove) {
	    H = csmTime2Hub(csm,dTime);
	    a = csmTime2Exp(csm,dTime);
	    }
	else {
	    H = 0.0;
	    a = 1.0;
	    }
	}
    /*    ~DistStellarFeedbackSmoothParams() {
	delete fb;
	}*/
    PUPable_decl(DistStellarFeedbackSmoothParams);
    DistStellarFeedbackSmoothParams(CkMigrateMessage *m) : SmoothParams(m) {}
    virtual void pup(PUP::er &p) {
        SmoothParams::pup(p);//Call base class
	p|a;
	p|H;
	p|gamma;
	p|fb;
	p|dTime;
	}
    };

#endif

    