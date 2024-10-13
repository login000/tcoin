#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <sstream>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <array>
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>
#include <sys/stat.h>
#include <ctime>
#include <unistd.h>
#include "popen2.h"
//set to 1 to enable some debug std::cout statements
#define DEBUG 0

#include "pcoin_defs.cpp"

/* OLD VALUES */
/*
#define TCOIN_PATH "/home/login/tcoin"
#define TCOIN_MSG_PATH "/home/login/tcoin/messages/"
#define TCOIN_SALT_PATH "/home/login/tcoin/salts/"
#define TCOIN_PASS_PATH "/home/login/tcoin/passwords/"
#define TCOIN_PROG_ACT_PATH "/home/login/tcoin/program_accounting/"
#define PROG_ACT_W_SLASH "program_accounting/"
#define PCOIN_KEY_PATH_W_SLASH "/home/login/bin/pcoin_keys/"
#define PCOIN_NEW_KEY_CMD "/bin/cat /dev/urandom | /usr/bin/base64 | /usr/bin/head -c 64 | /usr/bin/tr '+' '-' | /usr/bin/tr '/' '_'"
#define LS_PCOIN_KEY_CMD "/bin/ls /home/login/bin/pcoin_keys"
#define TCOIN_CODEZ_PATH "/home/login/bin/tcoin_codez"
#define TCOIN_BIN_PATH_W_SPACE "/home/login/bin/tcoin "

#define PCOIN_BIN_PATH "/home/login/bin/pcoin"
#define PCOIN_BIN_PATH_W_SPACE "/home/login/bin/pcoin "
#define TCOIN_PATH_W_SLASH "/home/login/tcoin/"
#define TCOIN_SCRYPT_PATH "/home/login/bin/scrypt"
*/

#define LS_HOME_CMD "/bin/ls /home"
#define BIN_ECHO_CMD "/bin/echo $$"
#define CHMOD_PERMISSIONS ((S_IRUSR | S_IWUSR) & ~S_IRWXG & ~S_IRWXO)
#define CHMOD_PROGRAM_KEY_PERMISSIONS (S_IRUSR & ~S_IWUSR & ~S_IXUSR & ~S_IRWXG & ~S_IRWXO)
#ifndef KROWBAR_OFF
  #define KROWBAR_SCORE_PATH "/home/krowbar/Code/irc/data/tildescores.txt"
  #define JU_SCORE_PATH "/home/jmjl/dev/juju/data/tildescores.txt"
#endif
#ifndef DA_OFF
  #define TROIDO_DACOINS_CMD "cd /home/troido/daily_adventure/client/ && /home/troido/daily_adventure/client/daclient printinfo 2>&1 | /bin/grep -oP '(?<=\"Coins\", )\[[:digit:]]+'"
#endif
#ifndef MINERCOIN_OFF
  #define MINERCOIN_CMD_PRE_USERNAME "/bin/grep -oP '(?<=\"~"
  #define MINERCOIN_CMD_PRE_USERNAME2 "/bin/grep -oP '(?<=\""
  #define MINERCOIN_CMD_POST_USERNAME "\": )[[:digit:]]+' /home/minerobber/Code/minerbot/minercoin.json"
#endif
#define USERNAME_LENGTH_LIMIT 25
#define TCOIN_MSG_LENGTH_LIMIT 280

#define ERR_MAIN_SEND_TOO_FEW_ARGS 6
#define ERR_MAIN_SEND_TOO_MANY_ARGS 7
#define ERR_MAIN_SEND_TOO_MANY_ARGS_MSG 104
#define ERR_SILENTSEND 2
#define ERR_MAIN_MSG_TOO_LONG 19
#define ERR_UNKNOWN_ARG 3
#define ERR_TCOIN_TO_SELF 5
#define ERR_RECEIVER_BLOCKED 4
#define ERR_NEGATIVE_SEND_AMOUNT 2
#define ERR_INSUFFICIENT_FUNDS 3
#define ERR_RECEIVER_NOT_FOUND 1

#define ERR_PCOIN_KEY_REFRESH_FAILED 20

#define ERR_IN_GET_INTERNAL_BALANCE -1
#define ERR_IN_ADD_INTERNAL_BALANCE_GET_INTERNAL_TOTAL_OWED_FAILED -3
#define ERR_ADD_INTERNAL_BALANCE_VALUE_TO_ADD_UNFULFILLABLE_USING_OWN_CURRENT_FUNDS -1
#define ERR_ADD_INTERNAL_BALANCE_USERNAME_DOESNT_EXIST -2
#define ERR_ADD_INTERNAL_BALANCE_ADD_TO_SELF -4
#define ERR_ADD_FILE_VALUE_INSUFFICIENT_FUNDS 1
#define ERR_ADD_FILE_VALUE_FATAL 999
#define ERR_IN_GET_INTERNAL_TOTAL_OWED_SELF_PROGRAM_DOESNT_EXIST -1
#define ERR_GET_FILE_VALUE_COULDNT_OPEN_BASEFILE 1
#define ERR_ADD_FILE_VALUE_COULDNT_OPEN_BASEFILE 1
#define ERR_SEND_MESSAGE_RECEIVER_MSG_FILE_UNABLE_TO_BE_UPDATED_FATAL 101
#define ERR_SEND_MESSAGE_PROGRAM_RECEIVER_MSG_FILE_UNABLE_TO_BE_UPDATED_FATAL 103
#define ERR_SEND_MESSAGE_SENDER_MSG_FILE_UNABLE_TO_BE_UPDATED_FATAL 102
#define ERR_SEND_MESSAGE_SENDER_PROGRAM_MSG_FILE_UNABLE_TO_BE_UPDATED_FATAL 104

#define ERR_KEY_EMPTY 22
#define ERR_KEY_NOT_IN_USE 9
#define ERR_NO_ARGS 8
#define ERR_IN_MAIN_GET_INTERNAL_TOTAL_OWED_FAILED 18
#define ERR_INTERNAL_BALANCE_USERNAME_NOT_FOUND 17
#define ERR_INTERNAL_BALANCE_NO_USERNAME_SUPPLIED 10
#define ERR_INTERNAL_BALANCE_TOO_MANY_ARGS 11
#define ERR_ADD_INTERNAL_BALANCE_TOO_FEW_ARGS 12
#define ERR_ADD_INTERNAL_BALANCE_TOO_MANY_ARGS 13
#define ERR_IN_MAIN_ADD_INTERNAL_BALANCE_AMOUNT_LARGER_THAN_COVERABLE_BY_UNOWED_BALANCE 14
#define ERR_IN_MAIN_ADD_INTERNAL_BALANCE_AMOUNT_MAKING_USER_INTERNAL_BALANCE_NEGATIVE 16
#define ERR_IN_MAIN_ADD_INTERNAL_BALANCE_NO_SUCH_USERNAME_FOUND 15
#define ERR_IN_MAIN_ADD_INTERNAL_BALANCE_ADD_TO_SELF 21

void exit_program(const int error_number)
{
  // Cleanup to do before exiting the program

  // Finally, we can exit
  std::exit(error_number);
}

//custom function to convert ("abcd.de") to ("abcde")
long long int strtol100(const char* amount_str)
{
  long long int result = 0;
  int multiplier = 1;
  int i=0;
  if(amount_str[i]=='-')
  {
    multiplier = -1;
    ++i;
  }
  else if(amount_str[i]=='+')
  {
    ++i;
  }
  else if(amount_str[i]=='\0') //empty string
  {
    return (long long int)(0);
  }
  //before the decimal point
  while(amount_str[i]!='.')
  {
    if(amount_str[i]>='0' && amount_str[i]<='9')
      result = result*10 + ((long long int)(amount_str[i]) - (long long int)('0'));
    else if(amount_str[i]=='\0') //e.g. "500"
    {
      result *= (multiplier*100); //multiplied by 100 to get centitildecoins
      return result;
    }
    else //error
    {
      return (long long int)(0);
    }
    ++i;
  }
  //at decimal point
  ++i;
  //after decimal point (i.e., tenth's place)
  if(amount_str[i]>='0' && amount_str[i]<='9')
    result = result*100 + ((long long int)(amount_str[i]) - (long long int)('0'))*10;
  else if(amount_str[i]=='\0') //e.g. "500."
  {
    result *= (multiplier*100); //multiplied by 100 to get centitildecoins
    return result;
  }
  else //error
  {
    return (long long int)(0);
  }
  //before hundredth's place
  ++i;
  //at hundredth's place
  if(amount_str[i]>='0' && amount_str[i]<='9')
  {
    result += ((long long int)(amount_str[i]) - (long long int)('0'));
  }
  else if(amount_str[i]=='\0') //e.g. "500.3"
  {
    result *= multiplier;
    return result;
  }
  else //error
  {
    return (long long int)(0);
  }
  result *= multiplier;
  return result;
}

//custom function to convert integer string to long long int fast
long long int strtol_fast(const char* amount_str)
{
  long long int result = 0;
  int multiplier = 1;
  int i=0;
  if(amount_str[i]=='-')
  {
    multiplier = -1;
    ++i;
  }
  else if(amount_str[i]=='+')
  {
    ++i;
  }
  else if(amount_str[i]=='\0') //empty string
  {
    return (long long int)(0);
  }
  //before the end of the string
  while(amount_str[i]>='0' && amount_str[i]<='9')
  {
    result = result*10 + ((long long int)(amount_str[i]) - (long long int)('0'));
    ++i;
  }
  result *= multiplier;
  return result;
}

//string constant time compare (only checks for equality (return 0 if equal))
int strctcmp(const char*a, const char*b)
{
  if(!(*a) || !(*b)) //a or b are empty (start with a NULL character)
    return 1;

  int r = 0;
  for (; *a && *b; ++a, ++b)
  {
    r |= *a != *b;
  }
  return r;
}

std::string exec(const char* cmd) {
  int i=0;
  do
  {
    std::array<char, 128> buffer;
    std::string result;
    try
    {
      std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
      if (!pipe)
      {
        ++i;
        std::cout << "popen() failed - " << i << std::endl;
        continue;
      }

      while (!feof(pipe.get())) {
          if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
              result += buffer.data();
      }
      return result;
    }
    catch (const std::exception& e)
    {
      ++i;
      std::cout << "popen() failed - " << i << " (exception " << e.what() << ")" << std::endl;
      continue;
    }
  } while(i < 100);

  throw std::runtime_error("popen() failed!");
  return std::string(""); //dummy line, never executes
}

std::string exec2(const char* cmd, std::string input) {
  std::string data_length_cmd_str = std::string(cmd) + std::string(PIPED_WORD_COUNT_CMD);
  const char* data_length_cmd_cstr = data_length_cmd_str.c_str();
  std::string data_length_str = exec(data_length_cmd_cstr);
  long long int data_length = strtol_fast(data_length_str.c_str())+1;
  std::vector <char> buffer;
  buffer.reserve(data_length);
  std::string result;
  files_t *fp = popen2(cmd);
  if (!fp) throw std::runtime_error("popen2() failed!");

  fputs((input+std::string("\n")).c_str(), fp->in);
  std::fflush(fp->in);

  while (!feof(fp->out)) {
      if (fgets(buffer.data(), data_length, fp->out) != nullptr)
        result += buffer.data();
  }
  pclose2(fp);
  return result;
}

long long int get_file_value(const char* file_name)
{
  char* file_path = new char[strlen(file_name)+sizeof(TCOIN_PATH_W_SLASH)+4];
  std::strcpy(file_path, TCOIN_PATH_W_SLASH);
  std::strcat(file_path, file_name);
  std::strcat(file_path, ".txt");

  std::ifstream file(file_path);

  if(!file)
  {
    if(!strcmp(file_name, "base/base"))
    {
      std::cerr << "\nError! Could not open file at " << file_path << "!\n\n";
      exit_program(ERR_GET_FILE_VALUE_COULDNT_OPEN_BASEFILE);
    }
    else {
      std::cerr << "\nError! Could not open file at " << file_path << "! Assuming its internal content is \"0\\n\".\n\n";
      return (long long int)(0);
    }
  }

  std::ostringstream ss;
  ss << file.rdbuf();

  delete[] file_path;
  return strtol_fast(ss.str().c_str());
}

int add_file_value(const char* file_name, const long long int &value_to_add, const long long int &base_amount)
{
  char* file_path = new char[strlen(file_name)+sizeof(TCOIN_PATH_W_SLASH)+4];
  char* temp_file_path = new char[strlen(file_name)+sizeof(TCOIN_PATH_W_SLASH)+8];
  std::strcpy(file_path, TCOIN_PATH_W_SLASH);
  std::strcat(file_path, file_name);
  std::strcpy(temp_file_path, file_path);
  std::strcat(file_path, ".txt");
  std::strcat(temp_file_path, "_tmp");
  std::strcat(temp_file_path, ".txt");

  std::ifstream file(file_path);

  if(!file)
  {
    if(!strcmp(file_name, "base/base"))
    {
      std::cerr << "\nError! Could not open file at " << file_path << "!\n\n";
      file.close();
      delete[] file_path;
      delete[] temp_file_path;
      exit_program(ERR_ADD_FILE_VALUE_COULDNT_OPEN_BASEFILE);
    }
  }

  std::ostringstream ss;
  ss << file.rdbuf();

  long long int old_value = strtol_fast(ss.str().c_str());

  //sufficient funds check
  if(value_to_add < 0 && (old_value + base_amount + value_to_add < 0))
  {
    file.close();
    delete[] file_path;
    delete[] temp_file_path;
    return ERR_ADD_FILE_VALUE_INSUFFICIENT_FUNDS;
  }

  long long int new_value = old_value + value_to_add;

  // Writing new value to file
  file.close();


  std::ofstream file2(temp_file_path);
  file2 << new_value << "\n";
  file2.close();

  chmod(temp_file_path, CHMOD_PERMISSIONS);

  if(!file2) //error
  {
    std::cerr << "Fatal error " << ERR_ADD_FILE_VALUE_FATAL << ": the file \"" << file_name << "\" was unable to be updated. Please contact " << TCOIN_ERR_CONTACT_EMAIL << " to report this error (because it requires manual recovery).";
    exit_program(ERR_ADD_FILE_VALUE_FATAL);
  }
  else
  {
    std::remove(file_path);
    while(1)
    {
      if(!std::rename(temp_file_path, file_path))
      {
        chmod(file_path, CHMOD_PERMISSIONS);
        break;
      }
    }
  }

  delete[] file_path;
  delete[] temp_file_path;
  return 0;
}

std::string global_username;
std::string get_username()
{
  return global_username;
}
int set_username(const std::string &username)
{
  global_username.assign(username);
  return 0;
}

std::string formatted_amount(long long int const& amount, char const* appended_chars_default = "", char const* appended_chars_singular = "")
{
  std::ostringstream ss;

  bool is_non_negative = amount >= 0 ? true : false;
  if(!is_non_negative) //i.e., is negative
    ss << "-";
  if(((is_non_negative*2-1)*amount) % 100 == 0)
    ss << (is_non_negative*2-1)*amount/100;
  else if((is_non_negative*2-1)*amount % 100 < 10)
    ss << (is_non_negative*2-1)*amount/100 << ".0" << (is_non_negative*2-1)*amount % 100;
  else
    ss << (is_non_negative*2-1)*amount/100 << "." << (is_non_negative*2-1)*amount % 100;
  if(((is_non_negative*2-1)*amount == 100) && strcmp(appended_chars_singular, ""))
    ss << appended_chars_singular;
  else
    ss << appended_chars_default;

  std::string formatted_string(ss.str());
  return formatted_string;
}

void cout_formatted_amount(long long int const& amount, char const* appended_chars_default = "", char const* appended_chars_singular = "", bool negative_with_parentheses = false)
{
  bool amount_is_negative = (amount < 0);
  if(negative_with_parentheses && amount_is_negative) std::cout << "(";
  std::cout << formatted_amount(amount, appended_chars_default, appended_chars_singular);
  if(negative_with_parentheses && amount_is_negative) std::cout << ")";
}

long long int base_amount;
long long int user_amount;
long long int krowbar_amount[2]; //krowbar's tilde game amount
long long int minercoin_amount[2]; //minerobber's !minercoin game amount (tilded ~username entry and non-tilded username entry, both)

void show_breakdown(const long long int &amount0 = 0, char const* amount0_source = "", const long long int &amount1 = 0, char const* amount1_source = "", const long long int &amount2 = 0, char const* amount2_source = "", const long long int &amount3 = 0, char const* amount3_source = "", const long long int &amount4 = 0, char const* amount4_source = "", const long long int &amount5 = 0, char const* amount5_source = "")
{
  bool a0 = (amount0 != 0 && strcmp(amount0_source, ""));
  bool a1 = (amount1 != 0 && strcmp(amount1_source, ""));
  bool a2 = (amount2 != 0 && strcmp(amount2_source, ""));
  bool a3 = (amount3 != 0 && strcmp(amount3_source, ""));
  bool a4 = (amount4 != 0 && strcmp(amount4_source, ""));
  bool a5 = (amount5 != 0 && strcmp(amount5_source, ""));
  if(a0 || a1 || a2 || a3 || a4 || a5)
  {
    if(a0)
    {
      std::cout << amount0_source << ",";
      cout_formatted_amount(amount0);
      if(a1 || a2 || a3 || a4 || a5)
      {
        std::cout << ";";
      }
    }
    if(a1)
    {
      std::cout << amount1_source << ",";
      cout_formatted_amount(amount1);
      if(a2 || a3 || a4 || a5)
      {
        std::cout << ";";
      }
    }
    if(a2)
    {
      std::cout << amount2_source << ",";
      cout_formatted_amount(amount2);
      if(a3 || a4 || a5)
      {
        std::cout << ";";
      }
    }
    if(a3)
    {
      std::cout << amount3_source << ",";
      cout_formatted_amount(amount3);
      if(a4 || a5)
      {
        std::cout << ";";
      }
    }
    if(a4)
    {
      std::cout << amount4_source << ",";
      cout_formatted_amount(amount4);
      if(a5)
      {
        std::cout << ";";
      }
    }
    if(a5)
    {
      std::cout << amount5_source << ",";
      cout_formatted_amount(amount5);
    }
    std::cout << "\n";
  }
}

void show_messages(const char* username)
{
  std::string messages_path = std::string(TCOIN_MSG_PATH) + std::string(username) + std::string("_messages.txt");
  std::ifstream fin(messages_path.c_str());
  char ch;
  bool first_char_is_newline = false;
  bool reached_eof = false;
  for(int i=0; i < 2; ++i)
  {
    if(ch = fin.get())
    {
      if(ch == std::istream::traits_type::eof()) //https://stackoverflow.com/questions/4533063/how-does-ifstreams-eof-work
      {
        if(first_char_is_newline && i==1)
        {
          reached_eof = true;
          std::cout << "No messages found.\n";
        }
        break;
      }
      else
      {
        if(i==0 && ch=='\n')
        {
          first_char_is_newline = true;
        }
        std::cout << ch;
      }
    }
  }
  if(!reached_eof)
  {
    std::cout << fin.rdbuf();
  }

  //removing eofbit
  fin.clear();
  //moving back two places from the end to read the last two characters
  fin.seekg(-2, std::ios::end);
  char chs[2]; //chs = characters
  chs[0] = fin.get();
  chs[1] = fin.get();
  fin.get(); //to set eofbit again because I like it to be just the way it was before removing the eofbit
  if(chs[0]!='\n' && chs[1]=='\n') //if only one newline at the end of the file
    std::cout << "\n"; //print another one

  fin.close();
}

void show_tsv_messages(const char* username) //tab-separated-value messages (tsv)
{
  std::string messages_path = std::string(TCOIN_PROG_ACT_PATH) + std::string(username) + std::string("/_MESSAGES.txt");
  std::ifstream fin(messages_path.c_str());
  std::cout << fin.rdbuf();
  std::cout << "\n";
  fin.close();
}

void show_messages_tail(const char* username, int lineCount)
{
  size_t const granularity = 100 * lineCount;
  std::string messages_path = std::string(TCOIN_MSG_PATH) + std::string(username) + std::string("_messages.txt");
  std::ifstream source(messages_path.c_str(), std::ios_base::binary);
  source.seekg(0, std::ios_base::end);
  size_t size = static_cast<size_t>(source.tellg());
  std::vector<char> buffer;
  int newlineCount = 0; //pseudo newline count
  while(source && buffer.size() != size && newlineCount <= lineCount)
  {
    buffer.resize(std::min(buffer.size() + granularity, size));
    source.seekg(-static_cast<std::streamoff>(buffer.size()), std::ios_base::end);
    source.read(buffer.data(), buffer.size());
    newlineCount = std::count(buffer.begin(), buffer.end(), '\n');
    for(std::vector<char>::size_type i = 0; i < (buffer.size()-1); ++i)
      if(buffer[i] == '\n' && (buffer[i+1] == '\n' || buffer[i+1] == ' '))
      {
        newlineCount--; // An entry as follows: "<stuff>\n \_message>\n\n" must be treated as a single newline-ended line (and thus count 1 newline (not 3))
        ++i; //three consecutive newlines should not be "two pairs" of newlines
      }
  }
  std::vector<char>::iterator start = buffer.begin();
  while(newlineCount > lineCount)
  {
    start = std::find(start, buffer.end(), '\n') + 1;
    if(*start == ' ' || *start == '\n')
      continue; //we're counting cutting off a '\n ' (and '\n\n') as zero (and one) newline cut off because "<stuff>\n \_message>\n\n" is one message
    --newlineCount;
  }
  std::cout << "Last " << lineCount << " Messages:\n";
  std::vector<char>::iterator end = remove(start, buffer.end(), '\r');
  if((start == (end-1)) && (*(start) == '\n')) //there is only one character, and it is a newline (i.e.. no messages)
    std::cout << "\nNo messages found.\n\n";
  else
  {
    if(*(start) != '\n') //if it starts with a newline, don't put another one
      std::cout << "\n";
    std::cout << std::string(start, end);
    if(*(end-2) != '\n' && *(end-1) == '\n') //if it ends with two newlines, don't put another one
      std::cout << "\n";
  }
}

void show_tsv_messages_tail(const char* username, int lineCount)
{
  size_t const granularity = 100 * lineCount;
  std::string messages_path = std::string(TCOIN_PROG_ACT_PATH) + std::string(username) + std::string("/_MESSAGES.txt");
  std::ifstream source(messages_path.c_str(), std::ios_base::binary);
  source.seekg(0, std::ios_base::end);
  size_t size = static_cast<size_t>(source.tellg());
  std::vector<char> buffer;
  int newlineCount = 0; //pseudo newline count
  while(source && buffer.size() != size && newlineCount <= lineCount)
  {
    buffer.resize(std::min(buffer.size() + granularity, size));
    source.seekg(-static_cast<std::streamoff>(buffer.size()), std::ios_base::end);
    source.read(buffer.data(), buffer.size());
    newlineCount = std::count(buffer.begin(), buffer.end(), '\n');
  }
  std::vector<char>::iterator start = buffer.begin();
  while(newlineCount > lineCount)
  {
    start = std::find(start, buffer.end(), '\n') + 1;
    --newlineCount;
  }
  std::vector<char>::iterator end = remove(start, buffer.end(), '\r');
  std::cout << std::string(start, end);
}

bool program_exists(const char* username)
{
  char *program_key_path = new char[strlen(username) + sizeof(PCOIN_KEY_PATH_W_SLASH) + 4]; //sizeof counts NULL char at the end too
  std::strcpy(program_key_path, PCOIN_KEY_PATH_W_SLASH);
  std::strcat(program_key_path, username);
  std::strcat(program_key_path, ".txt");

  std::ifstream fin(program_key_path);
  bool return_value = false;

  if(!fin) //file doesn't exist
  {
    fin.close();
    delete[] program_key_path;

    return_value = false; //program account not found
  }
  else
  {
    fin.close();
    delete[] program_key_path;

    return_value = true; //program account found
  }

  return return_value;
}

std::string refresh_pcoin_key()
{
  std::string new_key;
  const std::string username = get_username();
  char* program_key_path = new char[username.length() + sizeof(PCOIN_KEY_PATH_W_SLASH) + 4]; //sizeof counts NULL char at the end too
  char* temp_program_key_path = new char[username.length() + sizeof(PCOIN_KEY_PATH_W_SLASH) + 8]; //sizeof counts NULL char at the end too
  char* temp2_program_key_path = new char[username.length() + sizeof(PCOIN_KEY_PATH_W_SLASH) + 9]; //sizeof counts NULL char at the end too

  std::strcpy(program_key_path, PCOIN_KEY_PATH_W_SLASH);
  std::strcat(program_key_path, username.c_str());
  std::strcpy(temp_program_key_path, program_key_path);
  std::strcpy(temp2_program_key_path, program_key_path);
  std::strcat(program_key_path, ".txt");
  std::strcat(temp_program_key_path, "_tmp.txt");
  std::strcat(temp2_program_key_path, "_tmp2.txt");

  std::ifstream fin(program_key_path);

  if(!fin)
  {
    delete[] program_key_path;
    delete[] temp_program_key_path;
    delete[] temp2_program_key_path;
    return std::string("n/a");
  }

  fin.close();

  if(!std::rename(program_key_path, temp_program_key_path))
  {
    chmod(temp_program_key_path, CHMOD_PERMISSIONS);
    std::ofstream fin2(temp2_program_key_path);

    if(!fin2)
    {
      fin2.close();
      while(1)
      {
        if(!std::rename(temp_program_key_path, program_key_path))
        {
          chmod(program_key_path, CHMOD_PROGRAM_KEY_PERMISSIONS);
          break;
        }
      }
      delete[] temp2_program_key_path;
      delete[] temp_program_key_path;
      delete[] program_key_path;
      return std::string("n/a");
    }

    new_key = exec(PCOIN_NEW_KEY_CMD);
    fin2 << new_key << "\n";
    fin2.close();
    chmod(temp2_program_key_path, CHMOD_PROGRAM_KEY_PERMISSIONS);

    while(1)
    {
      if(!std::rename(temp2_program_key_path, program_key_path))
      {
        chmod(program_key_path, CHMOD_PROGRAM_KEY_PERMISSIONS);
        while(1)
        {
          if(!std::remove(temp_program_key_path))
            break;
        }
        break;
      }
    }
    delete[] program_key_path;
    delete[] temp_program_key_path;
    delete[] temp2_program_key_path;
    return new_key;
  }
  else
  {
    delete[] program_key_path;
    delete[] temp_program_key_path;
    delete[] temp2_program_key_path;
    return std::string("n/a");
  }

  delete[] program_key_path;
  delete[] temp_program_key_path;
  delete[] temp2_program_key_path;
  return std::string("n/a");
}

bool username_exists(const char* username)
{
  const static std::string all_usernames = exec(LS_HOME_CMD);
  std::istringstream iss(all_usernames);
  static std::vector<std::string> usernames{std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{}};
  if(std::find(usernames.begin(), usernames.end(), username) != usernames.end())
  {
    return true;
  }
  return program_exists(username);
}

bool file_is_empty(std::ifstream& pFile)
{
    return pFile.peek() == std::ifstream::traits_type::eof();
}

bool files_are_same(const char* file_path1, const char* file_path2)
{
  std::ifstream fin1(file_path1);
  if(!fin1)
    return false;
  std::ifstream fin2(file_path2);
  if(!fin2)
    return false;

  char c1;
  char c2;
  while(fin1.get(c1) && fin2.get(c2)) //we need to go inside the loop body even if one of the reads fails, so we use || instead of &&
  {
    if(c2 != c1)
      return false;
  }
  fin2.get(c2); //when fin1.get(c1) fails, fin2.get(c2) is not executed because of short-circuited boolean operators. This line compensates for that.
  if(fin1 || fin2) //one of the files must still be valid while both files are not simulatenously valid (after the while loop), which means the files are of different sizes
    return false;

  return true;
  //Because of (!fin1 && fin2) || (fin1 && !fin2), if any one
  //file is larger than the other, the files are deemed not the same.
  //If the last characters of the two files have not been read, then
  //both "fin1" and "fin2" will return true in the next iteration, and
  //in the current iteration, c1 and c2 have valid values that can be compared.
  //If the last characters of the two files were read, then "fin1" and "fin2"
  //will still return true until the next time fin1.get() or fin2.get() is called.
  //c1 and c2 still carry valid values (namely, the last characters of fin1 and fin2)
  //which are compared. At the next iteration, both fin1 and fin2 fail and the loop exits.
  //This means all c1's and c2's were equal in the iterations before. Thus, the two files are
  //deemed the same.
}

int send_message(const char* sender_username, const char* receiver_username, const char* message, const long long int &amount_sent, const char* option)
{
  std::string random_string = std::to_string(rand());

  char *receiver_path = new char[strlen(receiver_username) + sizeof(TCOIN_MSG_PATH) + 13]; //sizeof() includes '\0'
  char *temp_receiver_path = new char[strlen(receiver_username) + strlen(random_string.c_str()) + sizeof(TCOIN_MSG_PATH) + 13];

  std::strcpy(receiver_path, TCOIN_MSG_PATH);
  std::strcat(receiver_path, receiver_username);
  std::strcat(receiver_path, "_messages.txt"); // length = 13

  std::strcpy(temp_receiver_path, TCOIN_MSG_PATH);
  std::strcat(temp_receiver_path, receiver_username);
  std::strcat(temp_receiver_path, random_string.c_str());
  std::strcat(temp_receiver_path, "_messages.txt"); // length = 13

  //create receiver's message file if none exists
  //the message will be included in the receiver's
  //account when she/he initialises her/his account
  //at a later time

  char *receiver_salt_path = new char[strlen(receiver_username) + sizeof(TCOIN_SALT_PATH) + 9];
  char *receiver_salt_logged_in_path = new char[strlen(receiver_username) + sizeof(TCOIN_SALT_PATH) + 19];
  std::strcpy(receiver_salt_path, TCOIN_SALT_PATH);
  std::strcat(receiver_salt_path, receiver_username);
  std::strcpy(receiver_salt_logged_in_path, receiver_salt_path);
  std::strcat(receiver_salt_path, "_salt.txt"); // length = 9
  std::strcat(receiver_salt_logged_in_path, "_salt_logged_in.txt"); // length = 19

  std::ifstream fin(receiver_path);
  std::ifstream fin2(receiver_salt_path);
  std::ifstream fin3(receiver_salt_logged_in_path);

  if((!fin || file_is_empty(fin)) && ((!program_exists(receiver_username)) || ((!fin2 || file_is_empty(fin2)) && (!fin3 || file_is_empty(fin3)))))
  {
    fin.close();
    std::ofstream fout(receiver_path, std::fstream::trunc);
    fout << "\n";
    fout.close();
  }
  else
    fin.close();
  fin2.close();
  fin3.close();

  chmod(receiver_path, CHMOD_PERMISSIONS);

  delete[] receiver_salt_path;
  delete[] receiver_salt_logged_in_path;

  while(1)
  {
    if(!std::rename(receiver_path, temp_receiver_path))
    {
      char *really_temp_receiver_path = new char[strlen(temp_receiver_path) + 5];
      std::strcpy(really_temp_receiver_path, temp_receiver_path);
      std::strcat(really_temp_receiver_path, "_tmp");

      std::ifstream fin(temp_receiver_path);
      std::ofstream fout(really_temp_receiver_path);

      if(!file_is_empty(fin))
        fout << fin.rdbuf();
      fin.close();

      time_t now = std::time(NULL);
      char dt[26];
      #ifndef TILDEINSTITUTE
        std::strftime(dt, 26, "%a %b %_d %T %Y", std::gmtime(&now));
      #else
        std::strftime(dt, 26, "%a %b %e %T %Y", std::gmtime(&now));
      #endif
      char sender_formatted_string[26];
      char sender_arrow_formatted_string[47];
      char sender_arrow_string[47];
      char receiver_formatted_string[26];
      std::snprintf(sender_formatted_string, 26, "%25s", sender_username);
      std::snprintf(receiver_formatted_string, 26, "%-25s", receiver_username);
      int sender_username_length = std::strlen(sender_username);
      int number_of_chars = 26 >= sender_username_length ? 26 : sender_username_length;
      std::strncpy(sender_arrow_string, sender_username, number_of_chars);
      sender_arrow_string[number_of_chars] = '\0';
      std::strcat(sender_arrow_string, " ----");
      std::string amount_sent_str = formatted_amount(amount_sent);
      std::strncat(sender_arrow_string, amount_sent_str.c_str(), 10);
      std::strcat(sender_arrow_string, "----> ");
      std::snprintf(sender_arrow_formatted_string, 47, "%46s", sender_arrow_string);
      fout << dt << ": " << sender_arrow_formatted_string << receiver_username;
      if(!strcmp(message, "")) //if message is empty
        fout << "\n";
      else
      {
        fout << "\n \\_ " << sender_username << " said: ";
        for(int i=0; message[i]!='\0'; ++i)
        {
          if(message[i] == '\n')
            fout << "<new-line>"; //return-key pressed
          else if(message[i] == '\v')
            fout << "<vertical-tab>"; //vertical-tab symbol entered
          else
            fout << message[i];
        }
        fout << "\n\n";
      }
      fout.close();
      chmod(really_temp_receiver_path, CHMOD_PERMISSIONS);

      if(!fout) //error
      {
        std::cerr << "Fatal error " << ERR_SEND_MESSAGE_RECEIVER_MSG_FILE_UNABLE_TO_BE_UPDATED_FATAL << ": the receiver message file was unable to be updated. Please contact " << TCOIN_ERR_CONTACT_EMAIL << " to report this error (because it requires manual recovery).";
        exit_program(ERR_SEND_MESSAGE_RECEIVER_MSG_FILE_UNABLE_TO_BE_UPDATED_FATAL);
      }
      else
      {
        std::remove(temp_receiver_path);
      }

      while(1)
      {
        if(!std::rename(really_temp_receiver_path, temp_receiver_path))
          break;
      }

      // unlock_receiver_messages
      while(1)
      {
        if(!std::rename(temp_receiver_path, receiver_path))
          break;
      }

      chmod(receiver_path, CHMOD_PERMISSIONS);

      delete[] really_temp_receiver_path;
      delete[] temp_receiver_path;
      delete[] receiver_path;

      //additional place to write if sending to a program:
      if(program_exists(receiver_username))
      {
        random_string = std::string("rand");
        std::string program_receiver_path = std::string(TCOIN_PROG_ACT_PATH) + std::string(receiver_username) + std::string("/_MESSAGES.txt");
        std::string temp_program_receiver_path = std::string(TCOIN_PROG_ACT_PATH) + std::string(receiver_username) + std::string("/_MESSAGES") + random_string + std::string(".txt");
        std::string really_temp_program_receiver_path = temp_program_receiver_path + std::string("_tmp");

        //create program receiver's _MESSAGES file if none exists
        {
          std::ifstream fin(program_receiver_path.c_str());
          std::ifstream fin2(temp_program_receiver_path.c_str());
          if((!fin || file_is_empty(fin)) && (!fin2))
          {
            fin.close();
            std::ofstream fout(program_receiver_path.c_str(), std::fstream::trunc);
            fout << "\n";
            fout.close();
          }
          else
            fin.close();
          fin2.close();
          chmod(program_receiver_path.c_str(), CHMOD_PERMISSIONS);
        }

        while(1)
        {
          if(!std::rename(program_receiver_path.c_str(), temp_program_receiver_path.c_str()))
          {
            std::ifstream fin(temp_program_receiver_path.c_str());
            std::ofstream fout(really_temp_program_receiver_path.c_str());

            if(!file_is_empty(fin))
              fout << fin.rdbuf();
            fin.close();

            //now, sender_username, receiver_username, amount_sent
            char sender_formatted_string[26];
            char receiver_formatted_string[26];
            std::snprintf(sender_formatted_string, 26, "%s", sender_username);
            std::snprintf(receiver_formatted_string, 26, "%s", receiver_username);
            std::string amount_sent_str = formatted_amount(amount_sent);
            fout << now << "\t" << sender_formatted_string << "\t" << receiver_formatted_string << "\t" << amount_sent_str;
            if(!strcmp(message, "")) //if message is empty
              fout << "\n";
            else
            {
              fout << "\t" << sender_formatted_string << "\t";
              for(int i=0; message[i]!='\0'; ++i)
              {
                if(message[i] == '\n')
                  fout << "<new-line>"; //return-key pressed
                else if(message[i] == '\v')
                  fout << "<vertical-tab>"; //vertical-tab symbol entered
                else
                  fout << message[i];
              }
              fout << "\n";
            }
            fout.close();
            chmod(really_temp_program_receiver_path.c_str(), CHMOD_PERMISSIONS);

            if(!fout) //error
            {
              std::cerr << "Fatal error " << ERR_SEND_MESSAGE_PROGRAM_RECEIVER_MSG_FILE_UNABLE_TO_BE_UPDATED_FATAL << ": the receiver program_message file was unable to be updated. Please contact " TCOIN_ERR_CONTACT_EMAIL " to report this error (because it requires manual recovery).";
              exit_program(ERR_SEND_MESSAGE_PROGRAM_RECEIVER_MSG_FILE_UNABLE_TO_BE_UPDATED_FATAL);
            }
            else
            {
              std::remove(temp_program_receiver_path.c_str());
            }

            while(1)
            {
              if(!std::rename(really_temp_program_receiver_path.c_str(), temp_program_receiver_path.c_str()))
              break;
            }

            // unlock_receiver_program_messages
            while(1)
            {
              if(!std::rename(temp_program_receiver_path.c_str(), program_receiver_path.c_str()))
                break;
            }

            chmod(program_receiver_path.c_str(), CHMOD_PERMISSIONS);
            break;
          }//if statement with !std::rename for receiver's program accounting _messages file
        }//while loop for program accounting receiver's _messages file
      }//receiver is program account

      //locking sender_messages_after_receiver_messages_unlocked

      random_string = std::to_string(rand());

      char *sender_path = new char[strlen(sender_username) + sizeof(TCOIN_MSG_PATH) + 13];
      char *temp_sender_path = new char[strlen(sender_username) + strlen(random_string.c_str()) + sizeof(TCOIN_MSG_PATH) + 13];

      std::strcpy(sender_path, TCOIN_MSG_PATH);
      std::strcat(sender_path, sender_username);
      std::strcat(sender_path, "_messages.txt"); // length = 13

      std:strcpy(temp_sender_path, TCOIN_MSG_PATH);
      std::strcat(temp_sender_path, sender_username);
      std::strcat(temp_sender_path, random_string.c_str());
      std::strcat(temp_sender_path, "_messages.txt"); // length = 13

      while(1)
      {
        if(!std::rename(sender_path, temp_sender_path))
        {
          char *really_temp_sender_path = new char[strlen(temp_sender_path) + 5];
          std::strcpy(really_temp_sender_path, temp_sender_path);
          std::strcat(really_temp_sender_path, "_tmp");

          fin.open(temp_sender_path);
          fout.open(really_temp_sender_path);
          chmod(really_temp_sender_path, CHMOD_PERMISSIONS);

          fout << fin.rdbuf();

          now = std::time(NULL);
          #ifndef TILDEINSTITUTE
             std::strftime(dt, 26, "%a %b %_d %T %Y", std::gmtime(&now));
          #else
             std::strftime(dt, 26, "%a %b %e %T %Y", std::gmtime(&now));
          #endif
          char sender_formatted_string_right_aligned[26];
          char receiver_arrow_formatted_string[47];
          char receiver_arrow_string[47];
          std::snprintf(receiver_formatted_string, 26, "%25s", receiver_username);
          std::snprintf(sender_formatted_string, 26, "%-25s", sender_username);
          std::snprintf(sender_formatted_string_right_aligned, 26, "%25s", sender_username);
          int receiver_username_length = std::strlen(receiver_username);
          int number_of_chars = 26 >= receiver_username_length ? 26 : receiver_username_length;
          std::strncpy(receiver_arrow_string, receiver_username, number_of_chars);
          receiver_arrow_string[number_of_chars] = '\0';
          std::strcat(receiver_arrow_string, " <---");
          std::string amount_sent_str = formatted_amount(amount_sent);
          std::strncat(receiver_arrow_string, amount_sent_str.c_str(), 10);
          std::strcat(receiver_arrow_string, "----- ");
          std::snprintf(receiver_arrow_formatted_string, 47, "%46s", receiver_arrow_string);
          fout << dt << ": " << receiver_arrow_formatted_string << sender_username;
          if(!strcmp(message, "")) //if message is empty
            fout << "\n";
          else
          {
            fout << "\n \\_ " << sender_username << " said: ";
            for(int i=0; message[i]!='\0'; ++i)
            {
              if(message[i] == '\n')
                fout << "<new-line>"; //return-key pressed
              else if(message[i] == '\v')
                fout << "<vertical-tab>"; //vertical-tab symbol entered
              else
                fout << message[i];
            }
            fout << "\n\n";
          }

          fin.close();
          fout.close();

          if(!fout) //error
          {
            std::cerr << "Fatal error " << ERR_SEND_MESSAGE_SENDER_MSG_FILE_UNABLE_TO_BE_UPDATED_FATAL << ": the sender message file was unable to be updated. Please contact " << TCOIN_ERR_CONTACT_EMAIL << " to report this error (because it requires manual recovery).";
            exit_program(ERR_SEND_MESSAGE_SENDER_MSG_FILE_UNABLE_TO_BE_UPDATED_FATAL);
          }
          else
          {
            std::remove(temp_sender_path);
          }

          while(1)
          {
            if(!std::rename(really_temp_sender_path, temp_sender_path))
              break;
          }

          while(1)
          {
            if(!std::rename(temp_sender_path, sender_path))
              break;
          }

          delete[] really_temp_sender_path;
          delete[] temp_sender_path;
          delete[] sender_path;

          //additional place to write if sending from a program:
          if(program_exists(sender_username))
          {
            random_string = std::string("rand");
            std::string program_sender_path = std::string(TCOIN_PROG_ACT_PATH) + std::string(sender_username) + std::string("/_MESSAGES.txt");
            std::string temp_program_sender_path = std::string(TCOIN_PROG_ACT_PATH) + std::string(sender_username) + std::string("/_MESSAGES") + random_string + std::string(".txt");
            std::string really_temp_program_sender_path = temp_program_sender_path + std::string("_tmp");

            //create program sender's _MESSAGES file if none exists
            {
              std::ifstream fin(program_sender_path.c_str());
              std::ifstream fin2(temp_program_sender_path.c_str());
              if((!fin || file_is_empty(fin)) && (!fin2))
              {
                fin.close();
                std::ofstream fout(program_sender_path.c_str(), std::fstream::trunc);
                fout << "\n";
                fout.close();
              }
              else
                fin.close();
              fin2.close();
              chmod(program_sender_path.c_str(), CHMOD_PERMISSIONS);
            }

            while(1)
            {
              if(!std::rename(program_sender_path.c_str(), temp_program_sender_path.c_str()))
              {
                std::ifstream fin(temp_program_sender_path.c_str());
                std::ofstream fout(really_temp_program_sender_path.c_str());

                if(!file_is_empty(fin))
                  fout << fin.rdbuf();
                fin.close();

                //now (updated by code above to reflect "sending time"), sender_username, receiver_username, amount_sent
                char sender_formatted_string[26];
                char receiver_formatted_string[26];
                std::snprintf(sender_formatted_string, 26, "%s", sender_username);
                std::snprintf(receiver_formatted_string, 26, "%s", receiver_username);
                std::string amount_sent_str = formatted_amount(-1*amount_sent);
                fout << now << "\t" << receiver_formatted_string << "\t" << sender_formatted_string << "\t" << amount_sent_str;
                if(!strcmp(message, "")) //if message is empty
                  fout << "\n";
                else
                {
                  fout << "\t" << sender_formatted_string << "\t";
                  for(int i=0; message[i]!='\0'; ++i)
                  {
                    if(message[i] == '\n')
                      fout << "<new-line>"; //return-key pressed
                    else if(message[i] == '\v')
                      fout << "<vertical-tab>"; //vertical-tab symbol entered
                    else
                      fout << message[i];
                  }
                  fout << "\n";
                }
                fout.close();
                chmod(really_temp_program_sender_path.c_str(), CHMOD_PERMISSIONS);

                if(!fout) //error
                {
                  std::cerr << "Fatal error " << ERR_SEND_MESSAGE_SENDER_PROGRAM_MSG_FILE_UNABLE_TO_BE_UPDATED_FATAL << ": the sender program message file was unable to be updated. Please contact " << TCOIN_ERR_CONTACT_EMAIL << " to report this error (because it requires manual recovery).";
                  exit_program(ERR_SEND_MESSAGE_SENDER_PROGRAM_MSG_FILE_UNABLE_TO_BE_UPDATED_FATAL);
                }
                else
                {
                  std::remove(temp_program_sender_path.c_str());
                }

                while(1)
                {
                  if(!std::rename(really_temp_program_sender_path.c_str(), temp_program_sender_path.c_str()))
                  break;
                }

                // unlock_sender_program_messages
                while(1)
                {
                  if(!std::rename(temp_program_sender_path.c_str(), program_sender_path.c_str()))
                    break;
                }

                chmod(program_sender_path.c_str(), CHMOD_PERMISSIONS);
                break;
              }//if statement with !std::rename for sender's program accounting _messages file
            }//while loop for program accounting sender's _messages file
          }//sender is program account

          break;
        }
      }

      break;
    }
  }
  //finally, everything ran well and we can send the message to stdout if verbose is turned on
  if(!strcmp(option, "verbose") && strcmp(message, "")) //message should not be empty
  {
    //since the message to stdout from send() ends with "\n\n", we have commented this below line out
    //std::cout << "\n";
    std::cout << "In addition, the transaction message \"" << message << "\" was ";
    std::cout << "sent from `" << sender_username << "` to `" << receiver_username << "` successfully.";
    std::cout << "\n\n";
  }
  return 0;
}

bool user_is_locked(const char* username)
{
  std::ifstream fin((std::string(TCOIN_PATH_W_SLASH) + std::string(username) + std::string("_locked.txt")).c_str());
  if(!fin)
    return false;
  return true;
}

int send(const char* sender_username, const char* receiver_username, const long long int &amount_to_send, const long long int &base_amount, const char* option)
{
  int final_return_value = 0;

  //receiver usrname check
  if(username_exists(receiver_username))
  {
    if(!strcmp(sender_username, receiver_username))
    {
      std::cout << "\nSorry, you cannot send tildecoins to yourself.\n\n";
      return ERR_TCOIN_TO_SELF;
    }
    if(user_is_locked(receiver_username))
    {
      if(!strcmp(option, "verbose"))
        std::cout << "\nSorry, `" << receiver_username << "` does not wish to receive any tildecoins at this time.\n\n";
      return ERR_RECEIVER_BLOCKED;
    }
    if(amount_to_send <= 0)
    {
      if(!strcmp(option, "verbose"))
        std::cout << "\nSorry, that amount is not valid. The amount should be a positive decimal number when truncated to two decimal places.\n\n";
      return ERR_NEGATIVE_SEND_AMOUNT;
    }
    else
    {
      std::string random_string = std::to_string(rand());
      int return_value = -1;

      //additional place to deduct from if the sender is a program (which is
      //always the case when `pcoin` is used, but we'll check anyway
      //we do this before the "tcoin send" part so that if program tries to send
      //more than it owes receiver, it's checked before checking "tcoin send's"
      //conditions
      if(program_exists(sender_username))
      {
        random_string = std::string("rand"); //not really random
        std::string program_sender_path = std::string(TCOIN_PROG_ACT_PATH) + std::string(sender_username) + std::string("/") + std::string(receiver_username) + std::string(".txt");
        std::string temp_program_sender_path = std::string(TCOIN_PROG_ACT_PATH) + std::string(sender_username) + std::string("/") + std::string(receiver_username) + random_string + std::string(".txt");
        std::string temp_program_sender_username = std::string(PROG_ACT_W_SLASH) + std::string(sender_username) + std::string("/") + std::string(receiver_username) + random_string;

        //create program sender's "receiver's balance file" if none exists
        {
          std::ifstream fin(program_sender_path.c_str());
          std::ifstream fin2(temp_program_sender_path.c_str());
          #if DEBUG
            std::cout << program_sender_path << "," << temp_program_sender_path << "," << !fin << "," << file_is_empty(fin) << "," << !fin2 << std::endl;
          #endif
          if((!fin || file_is_empty(fin)) && (!fin2))
          {
            fin.close();
            std::ofstream fout(program_sender_path.c_str(), std::fstream::trunc);
            fout << "0\n";
            fout.close();
            #if DEBUG
              char dummy;
              std::cout << "Press enter to continue"; std::cin >> dummy;
            #endif
          }
          else
            fin.close();
          fin2.close();
          chmod(program_sender_path.c_str(), CHMOD_PERMISSIONS);
        }

        while(1)
        {
          if(!std::rename(program_sender_path.c_str(), temp_program_sender_path.c_str()))
          {
            //Insufficient funds check in add_file_value() itself
            //third argument, base_amount, is 0 because program
            //should not be allowed to send more than what is owed
            //to the receiver
            return_value = add_file_value(temp_program_sender_username.c_str(), -1 * amount_to_send, 0);

            //The same amount must also be deducted from the "_total.txt" file
            //which records the total amount owed to others
            if(return_value == 0)
            {
              random_string = std::string("rand");
              std::string program_sender_total_path = std::string(TCOIN_PROG_ACT_PATH) + std::string(sender_username) + std::string("/_TOTAL.txt");
              std::string temp_program_sender_total_path = std::string(TCOIN_PROG_ACT_PATH) + std::string(sender_username) + std::string("/_TOTAL") + random_string + std::string(".txt");
              std::string temp_program_sender_total_username = std::string(PROG_ACT_W_SLASH) + std::string(sender_username) + std::string("/_TOTAL") + random_string;

              //create program sender's "total balance file" if none exists
              {
                std::ifstream fin(program_sender_total_path.c_str());
                std::ifstream fin2(temp_program_sender_total_path.c_str());
                if((!fin || file_is_empty(fin)) && (!fin2))
                {
                  fin.close();
                  std::ofstream fout(program_sender_total_path.c_str(), std::fstream::trunc);
                  fout << "0\n";
                  fout.close();
                }
                else
                  fin.close();
                fin2.close();
                chmod(program_sender_total_path.c_str(), CHMOD_PERMISSIONS);
              }

              while(1)
              {
                if(!std::rename(program_sender_total_path.c_str(), temp_program_sender_total_path.c_str()))
                {
                  //Insufficient funds check in add_file_value() itself
                  //third argument, base_amount, is 0 because program
                  //should not be allowed to send more than what is owed
                  //to the receiver
                  add_file_value(temp_program_sender_total_username.c_str(), -1 * amount_to_send, 0);

                  random_string = std::to_string(rand());

                  // sizeof() includes '\0'
                  char* temp_sender_path = new char[strlen(sender_username) + strlen(random_string.c_str()) + sizeof(TCOIN_PATH_W_SLASH) + 4];
                  char* sender_path = new char[strlen(sender_username) + sizeof(TCOIN_PATH_W_SLASH) + 4];
                  char* temp_sender_username = new char[strlen(sender_username) + strlen(random_string.c_str()) + 1];

                  std::strcpy(temp_sender_username, sender_username);
                  std::strcat(temp_sender_username, random_string.c_str());

                  std::strcpy(temp_sender_path, TCOIN_PATH_W_SLASH);
                  std::strcat(temp_sender_path, temp_sender_username);
                  std::strcat(temp_sender_path, ".txt"); // length = 4

                  std::strcpy(sender_path, TCOIN_PATH_W_SLASH);
                  std::strcat(sender_path, sender_username);
                  std::strcat(sender_path, ".txt"); // length = 4

                  while(1)
                  {
                    if(!std::rename(sender_path, temp_sender_path))
                    {
                      //Insufficient funds check is in add_file_value()
                      return_value = add_file_value(temp_sender_username, -1 * amount_to_send, base_amount);

                      if(return_value == 0) // Funds sucessfully deducted from sender_username
                      {
                        random_string = std::string("rand");

                        // sizeof() includes '\0'
                        char *temp_receiver_path = new char[strlen(receiver_username) + strlen(random_string.c_str()) + sizeof(TCOIN_PATH_W_SLASH) + 4];
                        char *receiver_path = new char[strlen(receiver_username) + sizeof(TCOIN_PATH_W_SLASH) + 4];
                        char *temp_receiver_username = new char[strlen(receiver_username) + strlen(random_string.c_str()) + 1];

                        std::strcpy(temp_receiver_username, receiver_username);
                        std::strcat(temp_receiver_username, random_string.c_str());

                        std::strcpy(temp_receiver_path, TCOIN_PATH_W_SLASH);
                        std::strcat(temp_receiver_path, temp_receiver_username);
                        std::strcat(temp_receiver_path, ".txt");

                        std::strcpy(receiver_path, TCOIN_PATH_W_SLASH);
                        std::strcat(receiver_path, receiver_username);
                        std::strcat(receiver_path, ".txt");

                        //create receiver's balance file if none exists
                        //the balance will be included in the receiver's
                        //account when she/he initialises her/his account
                        //at a later time
                        char *receiver_salt_path = new char[strlen(receiver_username) + sizeof(TCOIN_SALT_PATH) + 9]; // sizeof() includes '\0'
                        char *receiver_salt_logged_in_path = new char[strlen(receiver_username) + sizeof(TCOIN_SALT_PATH) + 19];
                        std::strcpy(receiver_salt_path, TCOIN_SALT_PATH);
                        std::strcat(receiver_salt_path, receiver_username);
                        std::strcpy(receiver_salt_logged_in_path, receiver_salt_path);
                        std::strcat(receiver_salt_path, "_salt.txt"); // length = 9
                        std::strcat(receiver_salt_logged_in_path, "_salt_logged_in.txt"); // length = 19

                        std::ifstream fin(receiver_path);
                        std::ifstream fin2(receiver_salt_path);
                        std::ifstream fin3(receiver_salt_logged_in_path);

                        if((!fin || file_is_empty(fin)) && ((!program_exists(receiver_username)) || ((!fin2 || file_is_empty(fin2)) && (!fin3 || file_is_empty(fin3)))))
                        {
                          fin.close();
                          std::ofstream fout(receiver_path, std::fstream::trunc);
                          fout << "0\n";
                          fout.close();
                        }
                        else
                          fin.close();
                        fin2.close();
                        fin3.close();

                        chmod(receiver_path, CHMOD_PERMISSIONS);

                        delete[] receiver_salt_path;
                        delete[] receiver_salt_logged_in_path;

                        while(1)
                        {
                          if(!std::rename(receiver_path, temp_receiver_path))
                          {
                            //Insufficient funds check in add_file_value() itself
                            add_file_value(temp_receiver_username, amount_to_send, base_amount);

                            //additional place to write if sending to a program:
                            if(program_exists(receiver_username))
                            {
                              random_string = std::string("rand");
                              std::string program_receiver_path = std::string(TCOIN_PROG_ACT_PATH) + std::string(receiver_username) + std::string("/") + std::string(sender_username) + std::string(".txt");
                              std::string temp_program_receiver_path = std::string(TCOIN_PROG_ACT_PATH) + std::string(receiver_username) + std::string("/") + std::string(sender_username) + random_string + std::string(".txt");
                              std::string temp_program_receiver_username = std::string(PROG_ACT_W_SLASH) + std::string(receiver_username) + std::string("/") + std::string(sender_username) + random_string;

                              //create program receiver's balance file if none exists
                              {
                                std::ifstream fin(program_receiver_path.c_str());
                                std::ifstream fin2(temp_program_receiver_path.c_str());
                                if((!fin || file_is_empty(fin)) && (!fin2))
                                {
                                  fin.close();
                                  std::ofstream fout(program_receiver_path.c_str(), std::fstream::trunc);
                                  fout << "0\n";
                                  fout.close();
                                }
                                else
                                  fin.close();
                                fin2.close();
                                chmod(program_receiver_path.c_str(), CHMOD_PERMISSIONS);
                              }

                              while(1)
                              {
                                if(!std::rename(program_receiver_path.c_str(), temp_program_receiver_path.c_str()))
                                {
                                  //Insufficient funds check in add_file_value() itself
                                  add_file_value(temp_program_receiver_username.c_str(), amount_to_send, base_amount);

                                  //Value must also be added to a _total.txt file
                                  {
                                    random_string = std::string("rand");
                                    std::string program_receiver_total_path = std::string(TCOIN_PROG_ACT_PATH) + std::string(receiver_username) + std::string("/_TOTAL.txt");
                                    std::string temp_program_receiver_total_path = std::string(TCOIN_PROG_ACT_PATH) + std::string(receiver_username) + std::string("/_TOTAL")  + random_string + std::string(".txt");
                                    std::string temp_program_receiver_total_username = std::string(PROG_ACT_W_SLASH) + std::string(receiver_username) + std::string("/_TOTAL") + random_string;

                                    //create program receiver's "total balance file" if none exists
                                    {
                                      std::ifstream fin(program_receiver_total_path.c_str());
                                      std::ifstream fin2(temp_program_receiver_total_path.c_str());
                                      if((!fin || file_is_empty(fin)) && (!fin2))
                                      {
                                        fin.close();
                                        std::ofstream fout(program_receiver_total_path.c_str(), std::fstream::trunc);
                                        fout << "0\n";
                                        fout.close();
                                      }
                                      else
                                        fin.close();
                                      fin2.close();
                                      chmod(program_receiver_total_path.c_str(), CHMOD_PERMISSIONS);
                                    }

                                    while(1)
                                    {
                                      if(!std::rename(program_receiver_total_path.c_str(), temp_program_receiver_total_path.c_str()))
                                      {
                                        //Insufficient funds check in add_file_value() itself
                                        add_file_value(temp_program_receiver_total_username.c_str(), amount_to_send, base_amount);
                                        while(1)
                                        {
                                          if(!std::rename(temp_program_receiver_total_path.c_str(), program_receiver_total_path.c_str()))
                                            break;
                                        }
                                        break;
                                      }
                                    }
                                  }

                                  while(1)
                                  {
                                    if(!std::rename(temp_program_receiver_path.c_str(), program_receiver_path.c_str()))
                                      break;
                                  }
                                  break;
                                }
                              }
                            }

                            if(!strcmp(option, "verbose"))
                            {
                              std::cout << "\n";
                              cout_formatted_amount(amount_to_send, " tildecoins were ", " tildecoin was ");
                              std::cout << "sent from `" << sender_username << "` to `" << receiver_username << "`.";
                              std::cout << "\n\n";
                            }

                            while(1)
                            {
                              if(!std::rename(temp_receiver_path, receiver_path))
                                break;
                            }
                            delete[] temp_receiver_path;
                            delete[] receiver_path;
                            delete[] temp_receiver_username;
                            break;
                          }
                        }
                      }
                      else if(return_value == 1)
                      {
                        if(!strcmp(option, "verbose"))
                        {
                          long long int amount_of_funds = base_amount + get_file_value(temp_sender_username);
                          std::cout << "\nSorry, you do not have sufficient funds to execute this transaction. ";
                          std::cout << "Your current balance is ";
                          cout_formatted_amount(amount_of_funds, " tildecoins.\n\n", " tildecoin.\n\n");
                        }
                        final_return_value = ERR_INSUFFICIENT_FUNDS; //we don't simply "return 3" here because we want temp_sender_path to get renamed again
                      }

                      while(1)
                      {
                        if(!std::rename(temp_sender_path, sender_path))
                          break;
                      }

                      delete[] temp_sender_path;
                      delete[] sender_path;
                      delete[] temp_sender_username;
                      break;
                    }
                  }

                  while(1)
                  {
                    if(!std::rename(temp_program_sender_total_path.c_str(), program_sender_total_path.c_str()))
                      break;
                  }
                  break;
                }
              }
            }
            else if(return_value == 1)
            {
              long long int amount_owed = get_file_value(temp_program_sender_username.c_str());
              long long int amount_to_aib = (long long int)(amount_to_send) - amount_owed;
              std::cout << "\nSorry, you only owe `" << receiver_username << "` ";
              cout_formatted_amount(amount_owed, " tildecoins", " tildecoin");
              std::cout << ", not ";
              cout_formatted_amount(amount_to_send, " tildecoins. ", " tildecoin. ");
              std::cout << "Please run `" << PCOIN_BIN_PATH_W_SPACE << "add_internal_balance " << receiver_username << " ";
              cout_formatted_amount(amount_to_aib);
              std::cout << "` to sufficiently increase the amount owed to `" << receiver_username << "`.\n\n";

              final_return_value = ERR_INSUFFICIENT_FUNDS; //we don't simply "return 3" here because we want temp_program_sender_path to get renamed again
            }

            while(1)
            {
              if(!std::rename(temp_program_sender_path.c_str(), program_sender_path.c_str()))
                break;
            }
            break;
          }
        }
      }
    }
  }
  else
  {
    if(!strcmp(option, "verbose"))
      std::cout << "\nSorry, no user with the username `" << receiver_username << "` was found.\n\n";
    return ERR_RECEIVER_NOT_FOUND;
  }

  return final_return_value;
}

void help()
{
  std::cout <<"\npcoin is meant for programs. After each of the following commands, you will have to input a valid key to stdin.";
  std::cout << "\n`" << PCOIN_BIN_PATH_W_SPACE << "messages` or `" << PCOIN_BIN_PATH_W_SPACE << "-m`: check your messages";
  std::cout << "\n`" << PCOIN_BIN_PATH_W_SPACE << "messages <num>` or `" << PCOIN_BIN_PATH_W_SPACE << "-m <num>`: print the last <num> messages";
  std::cout << "\n`" << PCOIN_BIN_PATH_W_SPACE << "messages_tsv` or `" << PCOIN_BIN_PATH_W_SPACE << "-mtsv`: check your messages in tab-separated-values format";
  std::cout << "\n`" << PCOIN_BIN_PATH_W_SPACE << "messages_tsv <num>` or `" << PCOIN_BIN_PATH_W_SPACE << "-mtsv <num>`: print the last <num> messages in tab-separated-values format";
  std::cout << "\n`" << PCOIN_BIN_PATH_W_SPACE << "balance` or `" << PCOIN_BIN_PATH_W_SPACE << "-b`: print the number representing your balance";
  std::cout << "\n`" << PCOIN_BIN_PATH_W_SPACE << "total_owed` or `" << PCOIN_BIN_PATH_W_SPACE << "-to`: print the total amount owed to others";
  std::cout << "\n`" << PCOIN_BIN_PATH_W_SPACE << "internal_balance <username>` or `" << PCOIN_BIN_PATH_W_SPACE << "-ib <username>`: print the amount you owe <username>";
  std::cout << "\n`" << PCOIN_BIN_PATH_W_SPACE << "add_internal_balance <username>` or `" << PCOIN_BIN_PATH_W_SPACE << "-aib <username> <amount>`: add <amount> to the amount you owe <username>";
  std::cout << "\n`" << PCOIN_BIN_PATH_W_SPACE << "send <username> <amount>` or `" << PCOIN_BIN_PATH_W_SPACE  << "-s <username> <amount>`: send <amount> tildecoins to <username>";
  std::cout << "\n`" << PCOIN_BIN_PATH_W_SPACE << "send <username> <amount> \"<message>\"` or `" << PCOIN_BIN_PATH_W_SPACE << "-s <username> <amount> \"<message>\"`: optionally, include a message to be sent to <username>";
  std::cout << "\n`" << PCOIN_BIN_PATH_W_SPACE << "silentsend <username> <amount> [\"<message>\"]`, `" << PCOIN_BIN_PATH_W_SPACE << "send -s <username> <amount> [\"<message>\"]` or `" << PCOIN_BIN_PATH_W_SPACE << "-ss <username> <amount> [\"<message>\"]`: send <amount> tildecoins to <username> with an optional (as indicated by [ and ], which should not be included in the actual comment) message included without printing anything";
  std::cout << "\n`" << PCOIN_BIN_PATH_W_SPACE << "refresh_key` or `" << PCOIN_BIN_PATH_W_SPACE << "-rk`: generate a new key for your pcoin account and print it";
  std::cout << "\n`" << PCOIN_BIN_PATH_W_SPACE << "name` or `" << PCOIN_BIN_PATH_W_SPACE << "-n`: print the name on the account";
  std::cout << "\nIn the commands with `<username> <amount>`, switching the two arguments around (i.e., from `<username> <amount>` to `<amount> <username>`) will also work";
  std::cout << "\n`" << PCOIN_BIN_PATH_W_SPACE << "--help`, `" << PCOIN_BIN_PATH_W_SPACE << "help` or `" << PCOIN_BIN_PATH_W_SPACE << "-h`: print this help text";
  std::cout << "\nSend an email to " << TCOIN_PASS_RESET_CONTACT_EMAIL << " to report any errors or request a key for your program.\n\n";
}

bool is_number(const char* test_string)
{
    char* p;
    strtod(test_string, &p);
    return *p == 0;
}

std::string get_username_from_key(std::string &key)
{
  std::ifstream codefin(TCOIN_CODEZ_PATH);
  char code1[513], code2[513], code3[513];
  codefin >> code1;
  codefin >> code2;
  codefin >> code3;
  codefin.close();
  const static std::string all_usernames_dot_txt = exec2((std::string(TCOIN_BIN_PATH_W_SPACE) + std::string("pcoin_list")).c_str(), std::string(code3));

  std::istringstream iss(all_usernames_dot_txt);

  std::string word1, word2, program_name("n/a");
  //first word is program username with .txt on the end, second word is key
  while(iss >> word1)
  {
    char *program_key_path = new char[strlen(word1.c_str()) + sizeof(PCOIN_KEY_PATH_W_SLASH)]; //sizeof counts NULL char at the end too
    std::strcpy(program_key_path, PCOIN_KEY_PATH_W_SLASH);
    std::strcat(program_key_path, word1.c_str());

    std::ifstream fin(program_key_path);

    fin >> word2;

    if(!strctcmp(word2.c_str(), key.c_str()))
    {
      fin.close();
      delete[] program_key_path;
      program_name = word1.substr(0, word1.size()-4); //removing .txt from the username returned
    }
    else
    {
      fin.close();
      delete[] program_key_path;
    }
  }
  return program_name;
}

long long int get_internal_balance(const char* username)
{
  //sometimes, it just helps to double-check things
  //this makes the security of this function decoupled
  //from the 'deny access' mechanism in the "main"
  //function when an incorrect key is entered
  if(program_exists(get_username().c_str()) && username_exists(username))
  {
    std::string internal_path = std::string(TCOIN_PROG_ACT_PATH) + get_username() + std::string("/") + std::string(username) + std::string(".txt");
    //create internal balance file if none exists
    {
      std::ifstream fin(internal_path.c_str());
      if(!fin || file_is_empty(fin))
      {
        fin.close();
        std::ofstream fout(internal_path.c_str(), std::fstream::trunc);
        fout << "0\n";
        fout.close();
      }
      else
        fin.close();
      chmod(internal_path.c_str(), CHMOD_PERMISSIONS);
    }

    std::string internal_username = std::string(PROG_ACT_W_SLASH) + get_username() + std::string("/") + std::string(username);
    return get_file_value(internal_username.c_str());
  }
  return ERR_IN_GET_INTERNAL_BALANCE;
}

long long int get_internal_total_owed()
{
  if(program_exists(get_username().c_str()))
  {
    std::string internal_total_path = std::string(TCOIN_PROG_ACT_PATH) + get_username() + std::string("/_TOTAL.txt");
    //create internal total file if none exists
    {
      std::ifstream fin(internal_total_path.c_str());
      if(!fin || file_is_empty(fin))
      {
        fin.close();
        std::ofstream fout(internal_total_path.c_str(), std::fstream::trunc);
        fout << "0\n";
        fout.close();
      }
      else
        fin.close();
      chmod(internal_total_path.c_str(), CHMOD_PERMISSIONS);
    }

    std::string internal_total_username = std::string(PROG_ACT_W_SLASH) + get_username() + std::string("/_TOTAL");
    return get_file_value(internal_total_username.c_str());
  }
  return ERR_IN_GET_INTERNAL_TOTAL_OWED_SELF_PROGRAM_DOESNT_EXIST;
}

int add_internal_balance(const char* username, const long long int value_to_add)
{
  if(!strcmp(get_username().c_str(), username))
  {
    return ERR_ADD_INTERNAL_BALANCE_ADD_TO_SELF;
  }
  if(program_exists(get_username().c_str()) && username_exists(username))
  {
    std::string random_string = std::string("rand");
    std::string internal_username = std::string(PROG_ACT_W_SLASH) + get_username() + std::string("/") + std::string(username);
    std::string temp_internal_username = std::string(PROG_ACT_W_SLASH) + get_username() + std::string("/") + std::string(username) + random_string;

    long long int internal_total_owed = get_internal_total_owed();
    if(internal_total_owed == ERR_IN_GET_INTERNAL_TOTAL_OWED_SELF_PROGRAM_DOESNT_EXIST)
    {
      std::cerr << "\nError in add_internal_balance()! get_internal_total_owed() failed!\n\n";
      return ERR_IN_ADD_INTERNAL_BALANCE_GET_INTERNAL_TOTAL_OWED_FAILED;
    }

    if((value_to_add > 0) && (value_to_add > (base_amount + user_amount - internal_total_owed)))
    {
      return ERR_ADD_INTERNAL_BALANCE_VALUE_TO_ADD_UNFULFILLABLE_USING_OWN_CURRENT_FUNDS; //value_to_add is more than what the program can fulfil using its own current funds
    }

    std::string internal_path = std::string(TCOIN_PROG_ACT_PATH) + get_username() + std::string("/") + std::string(username) + std::string(".txt");
    std::string temp_internal_path = std::string(TCOIN_PROG_ACT_PATH) + get_username() + std::string("/") + std::string(username) + random_string + std::string(".txt");
    //create internal file if none exists
    {
      std::ifstream fin(internal_path.c_str());
      std::ifstream fin2(temp_internal_path.c_str());
      if((!fin || file_is_empty(fin)) && (!fin2))
      {
        fin.close();
        std::ofstream fout(internal_path.c_str(), std::fstream::trunc);
        fout << "0\n";
        fout.close();
      }
      else
        fin.close();
      fin2.close();
      chmod(internal_path.c_str(), CHMOD_PERMISSIONS);
    }

    int return_value;
    int final_return_value = 0;
    while(1)
    {
      if(!std::rename(internal_path.c_str(), temp_internal_path.c_str()))
      {
        return_value = add_file_value(temp_internal_username.c_str(), value_to_add, 0); //cannot make user's internal balance negative, so base_amount is 0

        if(return_value) //if return value is non-zero
        {
          final_return_value = return_value; //we don't simply "return return_value" because we want files to get renamed back to their original filenames
        }
        else //return value is zero, i.e., add_file_value succeeded
        //also need to update _total.txt
        {
          random_string = std::string("rand");
          std::string internal_total_username = std::string(PROG_ACT_W_SLASH) + get_username() + std::string("/_TOTAL");
          std::string temp_internal_total_username = std::string(PROG_ACT_W_SLASH) + get_username() + std::string("/_TOTAL") + random_string;
          std::string internal_total_path = std::string(TCOIN_PROG_ACT_PATH) + get_username() + std::string("/_TOTAL.txt");
          std::string temp_internal_total_path = std::string(TCOIN_PROG_ACT_PATH) + get_username() + std::string("/_TOTAL") + random_string + std::string(".txt");

          //create _total.txt file if none exists
          {
            std::ifstream fin(internal_total_path.c_str());
            std::ifstream fin2(temp_internal_total_path.c_str());
            if((!fin || file_is_empty(fin)) && (!fin2))
            {
              fin.close();
              std::ofstream fout(internal_total_path.c_str(), std::fstream::trunc);
              fout << "0\n";
              fout.close();
            }
            else
              fin.close();
            fin2.close();
            chmod(internal_total_path.c_str(), CHMOD_PERMISSIONS);
          }

          while(1)
          {
            if(!std::rename(internal_total_path.c_str(), temp_internal_total_path.c_str()))
            {
              return_value = add_file_value(temp_internal_total_username.c_str(), value_to_add, 0);
              while(1)
              {
                if(!std::rename(temp_internal_total_path.c_str(), internal_total_path.c_str()))
                  break;
              }
              break;
            }
          }
          if(return_value) //if return value is non-zero
            final_return_value = return_value; //we don't simply "return return_value" because we want files to get renamed back to their original filenames
        }

        while(1)
        {
          if(!std::rename(temp_internal_path.c_str(), internal_path.c_str()))
            break;
        }
        break;
      }
    }
    return final_return_value;
  }
  return ERR_ADD_INTERNAL_BALANCE_USERNAME_DOESNT_EXIST;
}

bool message_is_long(const char* test_string)
{
  for(int i=0; i < TCOIN_MSG_LENGTH_LIMIT+1; ++i)
    if(test_string[i] == '\0')
    {
      return false; //message is shorter than TCOIN_MSG_LENGTH_LIMIT characters
    }
  return true; //message is longer than TCOIN_MSG_LENGTH_LIMIT characters
}

int main(int argc, char *argv[])
{
  if(argc > 1 && (!strcmp(argv[1], "--help") || !strcmp(argv[1], "help") || !strcmp(argv[1], "-h")))
  {
    help();
    return 0;
  }

  {
    std::string key;
    std::string program_username;
    std::getline(std::cin, key);

    if(!key.compare(""))
    {
      std::cout << "\nSorry, you incorrectly specified an empty key.\n\n";
      return ERR_KEY_EMPTY;
    }

    program_username.assign(get_username_from_key(key));

    if(!program_username.compare("n/a"))
    {
      std::cout << "\nSorry, the key you specified is not in use.\n\n";
      return ERR_KEY_NOT_IN_USE;
    }
    set_username(program_username);
  }

  base_amount = 0;
  long long int unaltered_base_amount = base_amount;
  user_amount = 0;
  krowbar_amount[0] = krowbar_amount[1] = 0;
  minercoin_amount[0] = minercoin_amount[1] = 0;

  #ifndef KROWBAR_OFF
  //adding tildebot scores from krowbar to base amount
  {
    std::string line;
    const std::string username = get_username();
    const int username_length = username.length();

    std::string score_file_path;

    for(int j=0; j<2; ++j)
    {
      if(j == 0)
        score_file_path.assign(KROWBAR_SCORE_PATH);
      else if(j == 1)
        score_file_path.assign(JU_SCORE_PATH);

      std::ifstream fin(score_file_path);

      while(std::getline(fin, line))
      {
        char* line_c_string = new char[line.length()+1];
        std::strcpy(line_c_string, line.c_str());

        const int irc_username_length = username_length > USERNAME_LENGTH_LIMIT ? USERNAME_LENGTH_LIMIT : username_length;

        if(!strncasecmp(username.c_str(), line_c_string, irc_username_length)) //username starts with capital letter, but name in database does not
        {
          char number_of_tildes[21];
          number_of_tildes[0] = '0'; //just in case the loop below doesn't detect any digits
          number_of_tildes[1] = '\0';

          for(int i=0; i < 20; ++i)
          {
            if(std::isdigit(line_c_string[irc_username_length+3+i])
            || line_c_string[irc_username_length+3+i] == '-'
            || line_c_string[irc_username_length+3+i] == '.')
              number_of_tildes[i] = line_c_string[irc_username_length+3+i];
            else
            {
              number_of_tildes[i] = '\0'; //manually terminating the string
              break;
            }
          }
          number_of_tildes[20] = '\0'; //incase the number overflows 20 characters
          krowbar_amount[j] += strtol100(number_of_tildes);
          base_amount += krowbar_amount[j];
          //multiplied by 100 inside strtol100() to convert tildecoins to centitildecoins, which
          //is the unit used throughout the program (and converted appropriately when displayed)
        }
        delete[] line_c_string;
      }
    }
  }
  #endif

  #if DEBUG
    std::string debug_string("");
  #endif

  #ifndef MINERCOIN_OFF
  {
    std::string command_to_exec, minercoin_cmd_pre_username, number_of_tildes, username(get_username());
    username.at(0) = username.at(0)+'a'-'A';
    const std::string minercoin_cmd_post_username(MINERCOIN_CMD_POST_USERNAME), lowercase_username(username);
    //adding minercoin scores from minerobber to base amount (from "~username" and "username", both, in minerbot)
    #if DEBUG
      debug_string += lowercase_username;
    #endif
    for(int i=0; i<2; ++i)
    {
      if(i == 0)
        minercoin_cmd_pre_username.assign(MINERCOIN_CMD_PRE_USERNAME);
      else if(i == 1)
        minercoin_cmd_pre_username.assign(MINERCOIN_CMD_PRE_USERNAME2);

      command_to_exec = minercoin_cmd_pre_username + lowercase_username + minercoin_cmd_post_username;
      number_of_tildes = exec(command_to_exec.c_str());
      number_of_tildes.pop_back();
      //to get rid of the newline at the end
      if(is_number(number_of_tildes.c_str()))
      {
        minercoin_amount[i] += strtol100(number_of_tildes.c_str());
        base_amount += minercoin_amount[i];
      }
      //multiplied by 100 to convert tildecoins to centitildecoins, which
      //is the unit used throughout the program (and converted appropriately when displayed)
    }
  }
  #endif

  user_amount = get_file_value(get_username().c_str());

  srand((long int)(std::time(NULL)) + strtol_fast(exec(BIN_ECHO_CMD).c_str()));

  long long int total_amount = base_amount + user_amount;

  if(argc < 2)
  {
    std::cout << "\nSorry, `" << PCOIN_BIN_PATH << "` doesn't work. Please use `" << PCOIN_BIN_PATH_W_SPACE << "-m` for messages or `" << PCOIN_BIN_PATH_W_SPACE << "-b` to check your balance. `" << PCOIN_BIN_PATH_W_SPACE << "--help` prints the help text.\n\n";
    return ERR_NO_ARGS;
  }
  else if(!strcmp(argv[1], "breakdown") || !strcmp(argv[1], "-bd"))
  {
    std::cout << "total,";
    cout_formatted_amount(total_amount, ";", ";");
    show_breakdown(unaltered_base_amount, "baseamount", user_amount, "transfers", krowbar_amount[0], "tildegame", krowbar_amount[1], "jugame", minercoin_amount[0]+minercoin_amount[1], "minercoingame");
  }
  #if DEBUG
    else if(!strcmp(argv[1], "debug_string"))
    {
      std::cout << debug_string;
    }
  #endif
  else if(!strcmp(argv[1], "messages") || !strcmp(argv[1], "-m"))
  {
    double number_of_messages = 0.0;
    bool number_of_messages_is_specified = argc > 2 && is_number(argv[2]); //number of messages specified
    if(number_of_messages_is_specified)
      number_of_messages = std::strtod(argv[2], NULL);
    if(number_of_messages >= 1.0) //number of messages specified is a valid number
      show_messages_tail(get_username().c_str(), (long long int)(number_of_messages));
    else //show all messages
      show_messages(get_username().c_str());
  }
  else if(!strcmp(argv[1], "messages_tsv") || !strcmp(argv[1], "-mtsv"))
  {
    double number_of_messages = 0.0;
    bool number_of_messages_is_specified = argc > 2 && is_number(argv[2]); //number of messages specified
    if(number_of_messages_is_specified)
      number_of_messages = std::strtod(argv[2], NULL);
    if(number_of_messages >= 1.0) //number of messages specified is a valid number
      show_tsv_messages_tail(get_username().c_str(), (long long int)(number_of_messages));
    else //show all messages
      show_tsv_messages(get_username().c_str());
  }
  else if(!strcmp(argv[1], "balance") || !strcmp(argv[1], "-b"))
    cout_formatted_amount(total_amount, "\n");
  else if(!strcmp(argv[1], "total_owed") || !strcmp(argv[1], "-to"))
  {
    long long int total_owed = get_internal_total_owed();
    if(total_owed == ERR_IN_GET_INTERNAL_TOTAL_OWED_SELF_PROGRAM_DOESNT_EXIST)
    {
      std::cerr << "\nError in main()! get_internal_total_owed() failed!\n\n";
      return ERR_IN_MAIN_GET_INTERNAL_TOTAL_OWED_FAILED;
    }
    cout_formatted_amount(total_owed, "\n");
  }
  else if(!strcmp(argv[1], "internal_balance") || !strcmp(argv[1], "-ib"))
  {
    if(argc == 3) //second argument (the one right after "-ib") is the username
    {
      long long int internal_balance = get_internal_balance(argv[2]);
      if(internal_balance == ERR_IN_GET_INTERNAL_BALANCE) //username check doesn't pass
      {
        std::cout << "\nSorry, no user with the username `" << argv[2] << "` was found.\n\n";
        return ERR_INTERNAL_BALANCE_USERNAME_NOT_FOUND;
      }
      cout_formatted_amount(internal_balance, "\n");
    }
    else if(argc == 2) //no username supplied (too few arguments supplied)
    {
      std::cout << "\nSorry, too few command-line arguments were passed. The correct format is `" << PCOIN_BIN_PATH_W_SPACE << "internal_balance <username>`.\n\n";
      return ERR_INTERNAL_BALANCE_NO_USERNAME_SUPPLIED;
    }
    else if(argc > 3) //too many arguments supplied
    {
      std::cout << "\nSorry, too many command-line arguments were passed. The correct format is `" << PCOIN_BIN_PATH_W_SPACE << "internal_balance <username>`.\n\n";
      return ERR_INTERNAL_BALANCE_TOO_MANY_ARGS;
    }
  }
  else if(!strcmp(argv[1], "add_internal_balance") || !strcmp(argv[1], "-aib"))
  {
    if(argc < 4)
    {
      std::cout << "\nSorry, too few command-line arguments were passed. The correct format is `" << PCOIN_BIN_PATH_W_SPACE << "add_internal_balance <username> <amount>`.\n\n";
      return ERR_ADD_INTERNAL_BALANCE_TOO_FEW_ARGS;
    }
    else if(argc > 4)
    {
      std::cout << "\nSorry, too many command-line arguments were passed. The correct format is `" << PCOIN_BIN_PATH_W_SPACE << "add_internal_balance <username> <amount>`.\n\n";
      return ERR_ADD_INTERNAL_BALANCE_TOO_MANY_ARGS;
    }
    // number of arguments is exactly 3
    {
      int return_value = ERR_IN_ADD_INTERNAL_BALANCE_GET_INTERNAL_TOTAL_OWED_FAILED, return_value2 = ERR_IN_ADD_INTERNAL_BALANCE_GET_INTERNAL_TOTAL_OWED_FAILED;
      if(is_number(argv[3]))
        return_value = add_internal_balance(argv[2], strtol100(argv[3]));
      else
        return_value2 = add_internal_balance(argv[3], strtol100(argv[2]));

      if(return_value == ERR_ADD_INTERNAL_BALANCE_ADD_TO_SELF || return_value2 == ERR_ADD_INTERNAL_BALANCE_ADD_TO_SELF) //cannot add to self internal balance
      {
        std::cout << "\nSorry, you cannot add to your own internal balance.\n\n";
        return ERR_ADD_INTERNAL_BALANCE_ADD_TO_SELF;
      }
      if(return_value == ERR_ADD_INTERNAL_BALANCE_VALUE_TO_ADD_UNFULFILLABLE_USING_OWN_CURRENT_FUNDS || return_value2 == ERR_ADD_INTERNAL_BALANCE_VALUE_TO_ADD_UNFULFILLABLE_USING_OWN_CURRENT_FUNDS) //value_to_add was too large
      {
        std::cout << "\nSorry, the amount was larger than what the program's current unowed balance could cover.\n\n";
        return ERR_IN_MAIN_ADD_INTERNAL_BALANCE_AMOUNT_LARGER_THAN_COVERABLE_BY_UNOWED_BALANCE;
      }
      if(return_value == ERR_ADD_FILE_VALUE_INSUFFICIENT_FUNDS) //value_to_add was too negative
      {
        std::cout << "\nSorry, the amount was more negative than what `" << argv[2] << "` could cover.\n\n";
        return ERR_IN_MAIN_ADD_INTERNAL_BALANCE_AMOUNT_MAKING_USER_INTERNAL_BALANCE_NEGATIVE;
      }
      if(return_value2 == ERR_ADD_FILE_VALUE_INSUFFICIENT_FUNDS) //value_to_add was too negative
      {
        std::cout << "\nSorry, the amount was more negative than what `" << argv[3] << "` could cover.\n\n";
        return ERR_IN_MAIN_ADD_INTERNAL_BALANCE_AMOUNT_MAKING_USER_INTERNAL_BALANCE_NEGATIVE;
      }
      if(return_value == ERR_ADD_INTERNAL_BALANCE_USERNAME_DOESNT_EXIST) //username check doesn't pass
      {
        std::cout << "\nSorry, no user with the username `" << argv[2] << "` was found.\n\n";
        return ERR_IN_MAIN_ADD_INTERNAL_BALANCE_NO_SUCH_USERNAME_FOUND;
      }
      if(return_value2 == ERR_ADD_INTERNAL_BALANCE_USERNAME_DOESNT_EXIST) //username check doesn't pass
      {
        std::cout << "\nSorry, no user with the username `" << argv[3] << "` was found.\n\n";
        return ERR_IN_MAIN_ADD_INTERNAL_BALANCE_NO_SUCH_USERNAME_FOUND;
      }
    }
  }
  else if(!strcmp(argv[1], "send") || !strcmp(argv[1], "-s"))
  {
    if(argc == 5)
    {
      if(!strcmp(argv[2], "-s"))
      {
        int return_value;
        long long int amount = 0;
        char *receiver = NULL;

        if(is_number(argv[3]))
        {
          amount = strtol100(argv[3]);
          receiver = argv[4];
        }
        else
        {
          amount = strtol100(argv[4]);
          receiver = argv[3];
        }
        return_value = send(get_username().c_str(), receiver, amount, base_amount, "silent");
        if(!return_value) //send was successful
          send_message(get_username().c_str(), receiver, "", amount, "silent");
      }
      else //argument count is 5 because a custom message was included
      {
        int return_value;
        long long int amount = 0;
        char* receiver = NULL;

        return_value = message_is_long(argv[4]);
        if(return_value) //message is too long
        {
          std::cout << "\nSorry, the message was longer than " << TCOIN_MSG_LENGTH_LIMIT << " characters. Please keep messages at or below this limit.\n\n";
          return ERR_MAIN_MSG_TOO_LONG;
        }
        if(is_number(argv[2]))
        {
          amount = strtol100(argv[2]);
          receiver = argv[3];
        }
        else
        {
          amount = strtol100(argv[3]);
          receiver = argv[2];
        }
        return_value = send(get_username().c_str(), receiver, amount, base_amount, "verbose");
        if(!return_value) //send was successful
          send_message(get_username().c_str(), receiver, argv[4], amount, "verbose");
      }
    }
    else if(argc == 6)
    {
      if(!strcmp(argv[2], "-s"))
      { //argument count is 6 because of silent send with custom message included
        int return_value;
        long long int amount = 0;
        char *receiver = NULL;

        return_value = message_is_long(argv[5]);
        if(return_value) //message is too long
        {
          std::cout << "\nSorry, the message was longer than " << TCOIN_MSG_LENGTH_LIMIT << " characters. Please keep messages at or below this limit.\n\n";
          return ERR_MAIN_MSG_TOO_LONG;
        }

        if(is_number(argv[3]))
        {
          amount = strtol100(argv[3]);
          receiver = argv[4];
        }
        else
        {
          amount = strtol100(argv[4]);
          receiver = argv[3];
        }
        return_value = send(get_username().c_str(), receiver, amount, base_amount, "silent");
        if(!return_value) //send was successful
          send_message(get_username().c_str(), receiver, argv[5], amount, "silent");
      }
      else
      {
        //too many command-line arguments were passed (6 args) (probably a message was intended)
        std::cout << "\nSorry, too many command-line arguments were passed. The correct format is `tcoin send <username> <amount> \"<message>\"`.\n\n";
        return ERR_MAIN_SEND_TOO_MANY_ARGS_MSG;
      }
    }
    else if(argc < 4)
    {
      std::cout << "\nSorry, too few command-line arguments were passed. The correct format is `" << PCOIN_BIN_PATH_W_SPACE << "send <username> <amount>`.\n\n";
      return ERR_MAIN_SEND_TOO_FEW_ARGS;
    }
    else if(argc > 4)
    {
      std::cout << "\nSorry, too many command-line arguments were passed. The correct format is `" << PCOIN_BIN_PATH_W_SPACE << "send <username> <amount>`.\n\n";
      return ERR_MAIN_SEND_TOO_MANY_ARGS;
    }
    else
    {
      int return_value;
      long long int amount = 0;
      char* receiver = NULL;

      if(is_number(argv[2]))
      {
        amount = strtol100(argv[2]);
        receiver = argv[3];
      }
      else
      {
        amount = strtol100(argv[3]);
        receiver = argv[2];
      }
      return_value = send(get_username().c_str(), receiver, amount, base_amount, "verbose");
      if(!return_value) //send was successful
        send_message(get_username().c_str(), receiver, "", amount, "verbose");
    }
  }
  else if(!strcmp(argv[1], "silentsend") || !strcmp(argv[1], "-ss"))
  {
    if(argc==4)
    {
      int return_value;
      long long int amount = 0;
      char* receiver = NULL;

      if(is_number(argv[2]))
      {
        amount = strtol100(argv[2]);
        receiver = argv[3];
      }
      else
      {
        amount = strtol100(argv[3]);
        receiver = argv[2];
      }
      return_value = send(get_username().c_str(), receiver, amount, base_amount, "silent");
      if(!return_value) //send was successful
        send_message(get_username().c_str(), receiver, "", amount, "silent");
    }
    if(argc==5) //custom message included
    {
      int return_value;
      long long int amount = 0;
      char* receiver = NULL;

      return_value = message_is_long(argv[4]);
      if(return_value) //message is too long
      {
        std::cout << "\nSorry, the message was longer than " << TCOIN_MSG_LENGTH_LIMIT << " characters. Please keep messages at or below this limit.\n\n";
        return ERR_MAIN_MSG_TOO_LONG;
      }
      if(is_number(argv[2]))
      {
        amount = strtol100(argv[2]);
        receiver = argv[3];
      }
      else
      {
        amount = strtol100(argv[3]);
        receiver = argv[2];
      }
      return_value = send(get_username().c_str(), receiver, amount, base_amount, "silent");
      if(!return_value) //send was successful
        send_message(get_username().c_str(), receiver, argv[4], amount, "silent");
    }
    else
      return ERR_SILENTSEND;
  }
  else if(!strcmp(argv[1], "refresh_key") || !strcmp(argv[1], "-rk"))
  {
    std::string new_key;

    new_key.assign(refresh_pcoin_key());

    if(!new_key.compare("n/a") || new_key.length() != 64)
    {
      std::cout << "\nSorry, key was not refreshed.\n\n"; //make sure this is less than 64 characters
      return ERR_PCOIN_KEY_REFRESH_FAILED;
    }
    else
      std::cout << new_key << "\n";
  }
  else if(!strcmp(argv[1], "name") || !strcmp(argv[1], "-n"))
  {
    std::cout << get_username() << "\n";
  }
  else
  {
    std::cout << "\nSorry, an unknown command-line argument was received. `" << PCOIN_BIN_PATH_W_SPACE << "help` will print the help text.\n\n";
    return ERR_UNKNOWN_ARG;
  }

  return 0;
}
