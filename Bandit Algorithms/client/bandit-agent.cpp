#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <vector>
#include <random>
#include <string>

#include "gsl/gsl_rng.h"
#include "gsl/gsl_randist.h"

#define MAXHOSTNAME 256

using namespace std;

void options(){

  cout << "Usage:\n";
  cout << "bandit-agent\n"; 
  cout << "\t[--numArms numArms]\n";
  cout << "\t[--randomSeed randomSeed]\n";
  cout << "\t[--horizon horizon]\n";
  cout << "\t[--hostname hostname]\n";
  cout << "\t[--port port]\n";
  cout << "\t[--algorithm algorithm]\n";
  cout << "\t[--epsilon epsilon]\n";

}


/*
  Read command line arguments, and set the ones that are passed (the others remain default.)
*/
bool setRunParameters(int argc, char *argv[], int &numArms, int &randomSeed, unsigned long int &horizon, string &hostname, int &port, string &algorithm, double &epsilon){

  int ctr = 1;
  while(ctr < argc){

    //cout << string(argv[ctr]) << "\n";

    if(string(argv[ctr]) == "--help"){
      return false;//This should print options and exit.
    }
    else if(string(argv[ctr]) == "--numArms"){
      if(ctr == (argc - 1)){
	return false;
      }
      numArms = atoi(string(argv[ctr + 1]).c_str());
      ctr++;
    }
    else if(string(argv[ctr]) == "--randomSeed"){
      if(ctr == (argc - 1)){
	return false;
      }
      randomSeed = atoi(string(argv[ctr + 1]).c_str());
      ctr++;
    }
    else if(string(argv[ctr]) == "--horizon"){
      if(ctr == (argc - 1)){
	return false;
      }
      horizon = atoi(string(argv[ctr + 1]).c_str());
      ctr++;
    }
    else if(string(argv[ctr]) == "--hostname"){
      if(ctr == (argc - 1)){
	return false;
      }
      hostname = string(argv[ctr + 1]);
      ctr++;
    }
    else if(string(argv[ctr]) == "--port"){
      if(ctr == (argc - 1)){
	return false;
      }
      port = atoi(string(argv[ctr + 1]).c_str());
      ctr++;
    }
    else if(string(argv[ctr]) == "--algorithm"){
      if(ctr == (argc - 1)){
  return false;
      }
      algorithm = string(argv[ctr + 1]);
      ctr++;
    }
     else if(string(argv[ctr]) == "--epsilon"){
      if(ctr == (argc - 1)){
  return false;
      }
      epsilon = atof(string(argv[ctr + 1]).c_str());
      ctr++;
    }
    else{
      return false;
    }

    ctr++;
  }

  return true;
}


// Code for beta distribution 
// Source : https://stackoverflow.com/questions/15165202/random-number-generator-with-beta-distribution

namespace sftrabbit {

  template <typename RealType = double>
  class beta_distribution
  {
    public:
      typedef RealType result_type;

      class param_type
      {
        public:
          typedef beta_distribution distribution_type;

          explicit param_type(RealType a = 2.0, RealType b = 2.0)
            : a_param(a), b_param(b) { }

          RealType a() const { return a_param; }
          RealType b() const { return b_param; }

          bool operator==(const param_type& other) const
          {
            return (a_param == other.a_param &&
                    b_param == other.b_param);
          }

          bool operator!=(const param_type& other) const
          {
            return !(*this == other);
          }

        private:
          RealType a_param, b_param;
      };

      explicit beta_distribution(RealType a = 2.0, RealType b = 2.0)
        : a_gamma(a), b_gamma(b) { }
      explicit beta_distribution(const param_type& param)
        : a_gamma(param.a()), b_gamma(param.b()) { }

      void reset() { }

      param_type param() const
      {
        return param_type(a(), b());
      }

      void param(const param_type& param)
      {
        a_gamma = gamma_dist_type(param.a());
        b_gamma = gamma_dist_type(param.b());
      }

      template <typename URNG>
      result_type operator()(URNG& engine)
      {
        return generate(engine, a_gamma, b_gamma);
      }

      template <typename URNG>
      result_type operator()(URNG& engine, const param_type& param)
      {
        gamma_dist_type a_param_gamma(param.a()),
                        b_param_gamma(param.b());
        return generate(engine, a_param_gamma, b_param_gamma); 
      }

      result_type min() const { return 0.0; }
      result_type max() const { return 1.0; }

      result_type a() const { return a_gamma.alpha(); }
      result_type b() const { return b_gamma.alpha(); }

      bool operator==(const beta_distribution<result_type>& other) const
      {
        return (param() == other.param() &&
                a_gamma == other.a_gamma &&
                b_gamma == other.b_gamma);
      }

      bool operator!=(const beta_distribution<result_type>& other) const
      {
        return !(*this == other);
      }

    private:
      typedef std::gamma_distribution<result_type> gamma_dist_type;

      gamma_dist_type a_gamma, b_gamma;

      template <typename URNG>
      result_type generate(URNG& engine,
        gamma_dist_type& x_gamma,
        gamma_dist_type& y_gamma)
      {
        result_type x = x_gamma(engine);
        return x / (x + y_gamma(engine));
      }
  };

  template <typename CharT, typename RealType>
  std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os,
    const beta_distribution<RealType>& beta)
  {
    os << "~Beta(" << beta.a() << "," << beta.b() << ")";
    return os;
  }

  template <typename CharT, typename RealType>
  std::basic_istream<CharT>& operator>>(std::basic_istream<CharT>& is,
    beta_distribution<RealType>& beta)
  {
    std::string str;
    RealType a, b;
    if (std::getline(is, str, '(') && str == "~Beta" &&
        is >> a && is.get() == ',' && is >> b && is.get() == ')') {
      beta = beta_distribution<RealType>(a, b);
    } else {
      is.setstate(std::ios::failbit);
    }
    return is;
  }

}

const int MAXNUMARMS = 1000;
int pulls_tillnow[MAXNUMARMS]={0};
int success[MAXNUMARMS]={0};

double kl(double p, double q)
{
    if(p<1e-12)
      return log(1/(1-q));
    return p*log(p/q)+(1-p)*log((1-p)/(1-q));
}

double klfunction(int rew, int pulls, int param)
{
   double l = 1.0*rew/pulls;
   double r = 1.0;
   for(int i=1;i<=50;i++)
   {
      double mid = (l+r)/2;
      if(kl(1.0*rew/pulls,mid)*pulls<=param)
        l=mid;
      else r=mid;
   }
   return (l+r)/2;
}

/* ============================================================================= */
/* Write your algorithms here */
int sampleArm(string algorithm, double epsilon, int pulls, float reward, int numArms){
  if(algorithm.compare("rr") == 0){
    return(pulls % numArms);
  }
  
  else if(algorithm.compare("epsilon-greedy") == 0){
    if(pulls<numArms){
        return pulls;
    }
    int f = rand()%(100);
    bool explore = 1.0*f/100 < epsilon;

    if(explore){
      int i=rand()%numArms;
      // cerr(i);
      return i;
    }
    else{
      double highest_value =1.0*success[0]/pulls_tillnow[0], arm = 0;
      for(int i=1;i<numArms;i++){
        if(1.0*success[i]/pulls_tillnow[i]>highest_value){
          highest_value = 1.0*success[i]/pulls_tillnow[i];
          arm=i;
        }
      }
      return arm;
    }
  }
  else if(algorithm.compare("UCB") == 0){
    double * ucb_values =new double[MAXNUMARMS];
    if(pulls<numArms){
      return pulls;
    }
    for(int i=0;i<numArms;i++){
      ucb_values[i] = 1.0*success[i]/pulls_tillnow[i];
      ucb_values[i] += sqrt(2.0*log(pulls+1)/pulls_tillnow[i]);
 //     cerr<<ucb_values[i]<<" "<<i<<endl;
    }
    double highest_value =ucb_values[0], arm = 0;
    for(int i=1;i<numArms;i++){
      if(ucb_values[i]>highest_value){
        highest_value = ucb_values[i];
        arm=i;
      }
    }
    return arm;
  }
  else if(algorithm.compare("KL-UCB") == 0){
    if(pulls<numArms){
      return pulls;
    }
    // ideas from https://perso.limsi.fr/cappe/Research/Talks/11-gatsby.pdf
    double * kl_values = new double[MAXNUMARMS];
    for(int i=0;i<numArms;i++){
      kl_values[i]=klfunction(success[i],pulls_tillnow[i],log(pulls));
      // cerr<<kl_values[i]<<" "<<i<<endl;
    }
    double highest_value =kl_values[0], arm = 0;
    for(int i=1;i<numArms;i++){
      if(kl_values[i]>highest_value){
        highest_value = kl_values[i];
        arm=i;
      }
    }
    return arm;    
  }
  else if(algorithm.compare("Thompson-Sampling") == 0){
    double * beta_values = new double[MAXNUMARMS];
    for(int i=0;i<numArms;i++){
        std::random_device rd;
        std::mt19937 gen(rd());
        sftrabbit::beta_distribution<> beta(1+success[i], 1+pulls_tillnow[i]-success[i]);
        beta_values[i]=beta(gen);
    }
    double highest_value =beta_values[0], arm = 0;
    for(int i=1;i<numArms;i++){
      if(beta_values[i]>highest_value){
        highest_value = beta_values[i];
        arm=i;
      }
    }
    return arm;
  }
  else{
    return -1;
  }
}



/* ============================================================================= */


int main(int argc, char *argv[]){
  srand(time(NULL));
  // Run Parameter defaults.
  int numArms = 5;
  int randomSeed = time(0);
  unsigned long int horizon = 200;
  string hostname = "localhost";
  int port = 5000;
  string algorithm="random";
  double epsilon=0.0;

  //Set from command line, if any.
  if(!(setRunParameters(argc, argv, numArms, randomSeed, horizon, hostname, port, algorithm, epsilon))){
    //Error parsing command line.
    options();
    return 1;
  }

  struct sockaddr_in remoteSocketInfo;
  struct hostent *hPtr;
  int socketHandle;

  bzero(&remoteSocketInfo, sizeof(sockaddr_in));
  
  if((hPtr = gethostbyname((char*)(hostname.c_str()))) == NULL){
    cerr << "System DNS name resolution not configured properly." << "\n";
    cerr << "Error number: " << ECONNREFUSED << "\n";
    exit(EXIT_FAILURE);
  }

  if((socketHandle = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    close(socketHandle);
    exit(EXIT_FAILURE);
  }

  memcpy((char *)&remoteSocketInfo.sin_addr, hPtr->h_addr, hPtr->h_length);
  remoteSocketInfo.sin_family = AF_INET;
  remoteSocketInfo.sin_port = htons((u_short)port);

  if(connect(socketHandle, (struct sockaddr *)&remoteSocketInfo, sizeof(sockaddr_in)) < 0){
    //code added
    cout<<"connection problem"<<".\n";
    close(socketHandle);
    exit(EXIT_FAILURE);
  }


  char sendBuf[256];
  char recvBuf[256];

  float reward = 0;
  unsigned long int pulls=0;
  int armToPull = sampleArm(algorithm, epsilon, pulls, reward, numArms);
  
  sprintf(sendBuf, "%d", armToPull);

  cout << "Sending action " << armToPull << ".\n";
  while(send(socketHandle, sendBuf, strlen(sendBuf)+1, MSG_NOSIGNAL) >= 0){

    char temp;
    recv(socketHandle, recvBuf, 256, 0);
    sscanf(recvBuf, "%f %c %lu", &reward, &temp, &pulls);
    cout << "Received reward " << reward << ".\n";
    cout<<"Num of  pulls "<<pulls<<".\n";
    success[armToPull]+=reward;
    pulls_tillnow[armToPull]++;  

    armToPull = sampleArm(algorithm, epsilon, pulls, reward, numArms);

    sprintf(sendBuf, "%d", armToPull);
    cout << "Sending action " << armToPull << ".\n";
  }
  
  close(socketHandle);

  cout << "Terminating.\n";

  return 0;
}
          
