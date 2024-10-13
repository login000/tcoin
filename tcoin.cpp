#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <sstream>
#include <cstring>
#include <memory>
#include <limits>
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

#include "tcoin_defs.cpp"

/* OLD VALUES */
/*
#define TCOIN_PATH "/home/login/tcoin"
#define TCOIN_MSG_PATH "/home/login/tcoin/messages/"
#define TCOIN_SALT_PATH "/home/login/tcoin/salts/"
#define TCOIN_PASS_PATH "/home/login/tcoin/passwords/"
#define TCOIN_PROG_ACT_PATH "/home/login/tcoin/program_accounting/"
#define PROG_ACT_W_SLASH "program_accounting/"
#define PCOIN_KEY_PATH_W_SLASH "/home/login/bin/pcoin_keys/"
#define TCOIN_CODEZ_PATH "/home/login/bin/tcoin_codez"
#define TCOIN_BIN_PATH_W_SPACE "/home/login/bin/tcoin "
#define TCOIN_PATH_W_SLASH "/home/login/tcoin/"
#define TCOIN_SCRYPT_PATH "/home/login/bin/scrypt"
*/

#define LS_HOME_CMD "/bin/ls /home"
#define BIN_ECHO_CMD "/bin/echo $$"
#define CHMOD_PERMISSIONS ((S_IRUSR | S_IWUSR) & ~S_IRWXG & ~S_IRWXO)
#ifndef KROWBAR_OFF
  #define KROWBAR_SCORE_PATH "/home/krowbar/Code/irc/data/tildescores.txt"
  #define JU_SCORE_PATH "/home/jmjl/dev/juju/data/tildescores.txt"
#endif
#if !defined(TILDEINSTITUTE) && !defined(TILDEGURU)
  #define WHOAMI_PATH "/usr/bin/whoami"
#else
  #define WHOAMI_PATH "/usr/bin/getent passwd $(/usr/bin/id -ru) | /usr/bin/cut -d: -f1"
#endif
#ifndef DA_OFF
  #define TROIDO_DACOINS_CMD "cd /home/troido/daily_adventure/client/ && /home/troido/daily_adventure/client/daclient printinfo 2>&1 | /bin/grep -oP '(?<=\"Coins\", )[[:digit:]]+'"
#endif
#ifndef MINERCOIN_OFF
  #define MINERCOIN_CMD_PRE_USERNAME "/bin/grep -oP '(?<=\"~"
  #define MINERCOIN_CMD_PRE_USERNAME2 "/bin/grep -oP '(?<=\""
  #define MINERCOIN_CMD_POST_USERNAME "\": )[[:digit:]]+' /home/minerobber/Code/minerbot/minercoin.json"
#endif
#define USERNAME_LENGTH_LIMIT 25
#define TCOIN_MSG_LENGTH_LIMIT 280

#define ERR_NO_INIT 4
#define ERR_ALREADY_ON 40
#define ERR_NO_PWD_FILE 10
#define ERR_WRONG_PWD 20
#define ERR_WRONG_PWD2 30
#define ERR_ALREADY_OFF 20
#define ERR_DAMAGED_SALT_FILE 40
#define ERR_2_SALT_FILES 60
#define ERR_NOT_LOGGED_IN 5
#define ERR_MAIN_SEND_TOO_FEW_ARGS 6
#define ERR_MAIN_SEND_TOO_MANY_ARGS 7
#define ERR_MAIN_SEND_TOO_MANY_ARGS_MSG 104
#define ERR_MAIN_MSG_TOO_LONG 19
#define ERR_SILENTSEND 2
#define ERR_UNKNOWN_ARG 3
#define ERR_TCOIN_TO_SELF 5
#define ERR_RECEIVER_BLOCKED 4
#define ERR_NEGATIVE_SEND_AMOUNT 2
#define ERR_INSUFFICIENT_FUNDS 3
#define ERR_RECEIVER_NOT_FOUND 1
#define ERR_GET_FILE_VALUE_COULDNT_OPEN_BASEFILE 1
#define ERR_ADD_FILE_VALUE_COULDNT_OPEN_BASEFILE 1
#define ERR_ADD_FILE_VALUE_FATAL 999
#define ERR_GET_FILE_VALUE_COULDNT_OPEN_BASEFILE 1
#define ERR_ADD_FILE_VALUE_COULDNT_OPEN_BASEFILE 1
#define ERR_SEND_MESSAGE_RECEIVER_MSG_FILE_UNABLE_TO_BE_UPDATED_FATAL 101
#define ERR_SEND_MESSAGE_PROGRAM_RECEIVER_MSG_FILE_UNABLE_TO_BE_UPDATED_FATAL 103
#define ERR_SEND_MESSAGE_SENDER_MSG_FILE_UNABLE_TO_BE_UPDATED_FATAL 102
#define ERR_USER_IS_BLOCKED 105

void exit_program(const int error_number)
{
  // Cleanup to do before exiting the program

  //TODO: Print the actual error code to stderr

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
    {
      result = result*10 + ((long long int)(amount_str[i]) - (long long int)('0'));
    }
    else if(amount_str[i]=='\0') //e.g. 500
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
    result += ((long long int)(amount_str[i]) - (long long int)('0'));
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
  if(!(*a) || !(*b)) //*a or *b are empty (NULL characters)
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

std::string exec2_5(const char* cmd, std::string input, long long int data_length_override = -1) {
  long long int data_length = data_length_override;
  if(data_length_override == -1)
  {
    std::string data_length_cmd_str = std::string(cmd) + std::string(PIPED_WORD_COUNT_CMD);
    const char* data_length_cmd_cstr = data_length_cmd_str.c_str();
    std::string data_length_str = exec(data_length_cmd_cstr);
    data_length = strtol_fast(data_length_str.c_str())+1;
  }

  std::vector <char> buffer;

  if(data_length > 0)
  {
    buffer.reserve(data_length);
  }

  std::string result;

  files_t *fp = popen2(cmd);
  if (!fp) throw std::runtime_error("popen2() failed!");

  fputs((input+std::string("\n")).c_str(), fp->in);
  std::fflush(fp->in);

  if(data_length > 0)
  {
    while (!feof(fp->out)) {
      if (fgets(buffer.data(), data_length, fp->out) != nullptr)
        result += buffer.data();
    }
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
    else
    {
      std::cerr << "\nError! Could not open file at " << file_path << "! Assuming its internal content is \"0\\n\".\n\n";
      return (long long int)(0);
    }
  }

  std::ostringstream ss;
  ss << file.rdbuf();
  file.close();

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
    return 1;
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

std::string get_username()
{
  static std::string username;
  if(username.empty())
  {
    username = exec(WHOAMI_PATH);
    username.pop_back(); //to get rid of newline at the end
  }
  return username;
}

void num_stream_thousands_sep(std::ostringstream& ss, long long int const& amount, char sep)
{
  if(amount == 0)
  {
    ss << 0;
    return;
  }

  std::ostringstream rev;
  long long int reduced_amount = amount/1000;
  int residue = amount % 1000;
  bool residue_gte_100 = residue >= 100;
  bool residue_gte_10 = residue >= 10;
  bool residue_gte_1 = residue >= 1;

  int num_residue_digits = residue_gte_100 + residue_gte_10 + residue_gte_1;

  int mini_reduced_amount = residue/10;
  int mini_residue = residue % 10;

  do
  {
    for(int i=0; i<num_residue_digits; ++i)
    {
      rev << mini_residue;
      mini_residue = mini_reduced_amount % 10;
      mini_reduced_amount /= 10;
    }

    if(reduced_amount > 0)
    {
      for(int i=num_residue_digits; i<3; ++i)
      {
        rev << 0;
      }
      rev << sep;
    }

    residue = reduced_amount % 1000;
    residue_gte_100 = residue >= 100;
    residue_gte_10 = residue >= 10;
    residue_gte_1 = residue >= 1;

    num_residue_digits = residue_gte_100 + residue_gte_10 + residue_gte_1;
    mini_reduced_amount = residue/10;
    mini_residue = residue % 10;
    reduced_amount = reduced_amount/1000;
  } while(reduced_amount > 0);

  for(int i=0; i<num_residue_digits; ++i)
  {
    rev << mini_residue;
    mini_residue = mini_reduced_amount % 10;
    mini_reduced_amount /= 10;
  }

  std::string rev_str = rev.str();
  std::reverse(rev_str.begin(), rev_str.end());
  ss << rev_str;
}

bool stdout_is_piped = !isatty(fileno(stdout));

std::string formatted_amount(long long int const& amount, char const* appended_chars_default = "", char const* appended_chars_singular = "", char sep='\0')
{
  std::ostringstream ss;

  bool is_non_negative = amount >= 0 ? true : false;
  if(!is_non_negative) //i.e., is negative
    ss << "-";
  int amount_sign = is_non_negative*2-1;
  long long int abs_amount = amount_sign * amount;

  bool amount_is_integer = abs_amount % 100 == 0;
  bool amount_has_single_digit_cents = !amount_is_integer && (abs_amount % 100 < 10);
  bool amount_has_double_digit_cents = !(amount_is_integer || amount_has_single_digit_cents);

  if(stdout_is_piped || sep=='\0')
    ss << abs_amount/100;
  else
    num_stream_thousands_sep(ss, abs_amount/100, sep);

  if(amount_has_single_digit_cents)
    ss <<  ".0";
  if(amount_has_double_digit_cents)
    ss << ".";
  if(!amount_is_integer)
    ss << abs_amount % 100;

  if((abs_amount == 100) && strcmp(appended_chars_singular, ""))
    ss << appended_chars_singular;
  else
    ss << appended_chars_default;

  std::string formatted_string(ss.str());
  return formatted_string;
}

void cout_formatted_amount(long long int const& amount, char const* appended_chars_default = "", char const* appended_chars_singular = "", bool negative_with_parentheses = false, char sep='\'')
{
  bool amount_is_negative = (amount < 0);
  if(negative_with_parentheses && amount_is_negative) std::cout << "(";
  std::cout << formatted_amount(amount, appended_chars_default, appended_chars_singular, sep);
  if(negative_with_parentheses && amount_is_negative) std::cout << ")";
}

long long int base_amount;
long long int user_amount;
long long int krowbar_amount[2]; //krowbar's and ju's tilde game amount
long long int da_amount; //troido's daily adventure amount
long long int minercoin_amount[2]; //minerbot's minercoin game amount (tilded username (~username) and non-tilded username (username))

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
    std::cout << "Breakdown: ";
    if(a0)
    {
      cout_formatted_amount(amount0, "", "", true);
      std::cout << " [" << amount0_source << "]";
      if(a1 || a2 || a3 || a4 || a5)
      {
        std::cout << " + ";
      }
    }
    if(a1)
    {
      cout_formatted_amount(amount1, "", "", true);
      std::cout << " [" << amount1_source << "]";
      if(a2 || a3 || a4 || a5)
      {
        std::cout << " + ";
      }
    }
    if(a2)
    {
      cout_formatted_amount(amount2, "", "", true);
      std::cout << " [" << amount2_source << "]";
      if(a3 || a4 || a5)
      {
        std::cout << " + ";
      }
    }
    if(a3)
    {
      cout_formatted_amount(amount3, "", "", true);
      std::cout << " [" << amount3_source << "]";
      if(a4 || a5)
      {
        std::cout << " + ";
      }
    }
    if(a4)
    {
      cout_formatted_amount(amount4, "", "", true);
      std::cout << " [" << amount4_source << "]";
      if(a5)
      {
        std::cout << " + ";
      }
    }
    if(a5)
    {
      cout_formatted_amount(amount5, "", "", true);
      std::cout << " [" << amount5_source << "]";
    }
    std::cout << "\n";
  }
}

void show_balance(char const* username, const long long int &amount, const long long int &amount0 = 0, char const* amount0_source = "", const long long int &amount1 = 0, char const* amount1_source = "", const long long int &amount2 = 0, char const* amount2_source = "", const long long int &amount3 = 0, char const* amount3_source = "", const long long int &amount4 = 0, char const* amount4_source = "", const long long int &amount5 = 0, char const* amount5_source = "")
{
  std::cout << username << ", you have ";
  cout_formatted_amount(amount, " tildecoins", " tildecoin");
  std::cout << " to your name.\n\n";

  show_breakdown(amount0, amount0_source, amount1, amount1_source, amount2, amount2_source, amount3, amount3_source, amount4, amount4_source, amount5, amount5_source);

  std::cout << "\nThe command to send tildecoins to other users is `tcoin send <username> <amount>` or `tcoin -s <username> <amount>`.";
  std::cout << "\nThe command to log out of tildecoin is `tcoin off`.\n\n";
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
  while(fin1.get(c1) && fin2.get(c2))
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

void clear_messages(const char* username)
{
  std::string messages_path = std::string(TCOIN_MSG_PATH) + std::string(username) + std::string("_messages.txt");
  long long int max_long_long_int = std::numeric_limits<long long int>::max();

  //we don't actually delete, but just generate a backup ;)
  for(long long int i = 1; i <= max_long_long_int; ++i)
  {
    std::string messages_backup_path = std::string(TCOIN_MSG_PATH) + std::string(username) + std::string("_messages") + std::to_string(i) + std::string(".txt");
    std::ifstream fin(messages_backup_path.c_str());
    if(!fin || file_is_empty(fin)) //if file did not open or was empty, i.e., is available as a file name to backup (backups are incrementally numbered)
    {
      fin.close();
      rename(messages_path.c_str(), messages_backup_path.c_str());
      chmod(messages_backup_path.c_str(), CHMOD_PERMISSIONS);
      break;
    }
    else
      fin.close();
  }

  std::ofstream fout(messages_path.c_str(), std::fstream::trunc);
  fout << "\n";
  fout.close();
  chmod(messages_path.c_str(), CHMOD_PERMISSIONS);
}

void show_messages(const char* username)
{
  std::string messages_path = std::string(TCOIN_MSG_PATH) + std::string(username) + std::string("_messages.txt");
  std::ifstream fin(messages_path.c_str());
  std::cout << "Messages:\n";
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

bool username_exists(const char* username)
{
  const static std::string all_usernames = exec(LS_HOME_CMD);
  std::istringstream iss(all_usernames);
  static std::vector<std::string> usernames{std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{}};
  if(std::find(usernames.begin(), usernames.end(), username) != usernames.end())
  {
    return true;
  }

  //program usernames check
  return program_exists(username);
}

bool user_has_initialised(const char* username)
{
  bool return_value = true; //we assume the user has initialised and check for signs of that not being the case

  char *balance_file_path = new char[strlen(username) + sizeof(TCOIN_PATH_W_SLASH) + 4]; //sizeof counts NULL char at the end too
  std::strcpy(balance_file_path, TCOIN_PATH_W_SLASH);
  std::strcat(balance_file_path, username);
  std::strcat(balance_file_path, ".txt");

  char *messages_file_path = new char[strlen(username) + sizeof(TCOIN_MSG_PATH) + 13]; //sizeof counts NULL char at the end too
  std::strcpy(messages_file_path, TCOIN_MSG_PATH);
  std::strcat(messages_file_path, username);
  std::strcat(messages_file_path, "_messages.txt");

  char *password_file_path = new char[strlen(username) + sizeof(TCOIN_PASS_PATH) + 13]; //sizeof counts NULL char at the end too
  std::strcpy(password_file_path, TCOIN_PASS_PATH);
  std::strcat(password_file_path, username);
  std::strcat(password_file_path, "_password.txt");

  char *salt_file_path = new char[strlen(username) + sizeof(TCOIN_SALT_PATH) + 9]; //sizeof counts NULL char at the end too
  std::strcpy(salt_file_path, TCOIN_SALT_PATH);
  std::strcat(salt_file_path, username);
  std::strcat(salt_file_path, "_salt.txt");

  char *salt_logged_in_file_path = new char[strlen(username) + sizeof(TCOIN_SALT_PATH) + 19]; //sizeof counts NULL char at the end too
  std::strcpy(salt_logged_in_file_path, TCOIN_SALT_PATH);
  std::strcat(salt_logged_in_file_path, username);
  std::strcat(salt_logged_in_file_path, "_salt_logged_in.txt");

  std::ifstream fin1(balance_file_path);
  std::ifstream fin2(messages_file_path);
  std::ifstream fin3(password_file_path);
  std::ifstream fin4(salt_file_path);
  std::ifstream fin5(salt_logged_in_file_path);

  if(!fin1 || !fin2 || !fin3 || file_is_empty(fin3) || (!fin4 && !fin5) || (fin4 && file_is_empty(fin4)) || (fin5 && file_is_empty(fin5)))
    return_value = false; //user has not initialised completely

  fin1.close();
  fin2.close();
  fin3.close();
  fin4.close();
  fin5.close();

  delete[] balance_file_path;
  delete[] messages_file_path;
  delete[] password_file_path;
  delete[] salt_file_path;
  delete[] salt_logged_in_file_path;
  return return_value;
}

int log_off(const char* username)
{
  std::string salt_logged_in_file = std::string(TCOIN_SALT_PATH) + std::string(username) + std::string("_salt_logged_in.txt");
  std::string salt_file = std::string(TCOIN_SALT_PATH) + std::string(username) + std::string("_salt.txt");
  std::ifstream fin(salt_logged_in_file.c_str());
  if(!fin) //user is not currently logged in
  {
    std::cout << "\nSorry, you're already logged out. Thanks for being extra careful about being logged out before running untrusted programs (if you were going to do that).\n\n";
    return ERR_ALREADY_OFF;
  }
  else //user is logged in
  {
    if(file_is_empty(fin)) //user is logged in but salt_logged_in_file is empty
    {
      remove(salt_logged_in_file.c_str());
      remove(salt_file.c_str());
      std::cout << "\nSorry, your salt file is damaged. You will have to run `tcoin init` and create a new passphrase.\n\n";
      return ERR_DAMAGED_SALT_FILE;
    }
    else //user is logged in and salt_logged_in_file is not empty
    {
      fin.close();
      std::ifstream fin2(salt_file.c_str());
      if(!fin2 || (fin2 && file_is_empty(fin2))) //salt_file doesn't exist or is empty but salt_logged_in_file does
      {
        if(fin2 && file_is_empty(fin2))
          remove(salt_file.c_str());
        fin2.close();
        rename(salt_logged_in_file.c_str(), salt_file.c_str());
        std::cout << "\nYou have successfully logged out of tildecoin. Have a wonderful day!\n\n";
      }
      else //salt_file exists and is non-empty, and salt_file_logged_in also exists (!) and is non-empty
      {
        fin2.close();
        std::cout << "\nSorry, there's something seriously wrong with your salt files. You'll have to run `tcoin init` and create a new passphrase.\n\n";
        return ERR_2_SALT_FILES;
      }
    }
  }
  return 0;
}

bool is_logged_on(const char* username)
{
  std::string salt_logged_in_file = std::string(TCOIN_SALT_PATH) + std::string(username) + std::string("_salt_logged_in.txt");
  std::string salt_file = std::string(TCOIN_SALT_PATH) + std::string(username) + std::string("_salt.txt");
  std::ifstream fin(salt_logged_in_file.c_str());
  if(!fin)
    return false;
  else if(file_is_empty(fin))
  {
      fin.close();
      remove(salt_logged_in_file.c_str());
      remove(salt_file.c_str());
      std::cout << "\n\nYour salt logged in file is empty. You'll have to run `tcoin init` and create a new passphrase.\n\n";
      return false;
  }
  //control only reaches here if (fin && !file_is_empty(fin)), so the user is indeed logged in
  return true;
}

int log_on(const char* username)
{
  if(is_logged_on(username))
  {
    std::cout << "\nYou're already logged in. Please type `tcoin` to see your messages and balance.\n\n";
    return ERR_ALREADY_ON;
  }
  std::string salt_file = std::string(TCOIN_SALT_PATH) + std::string(username) + std::string("_salt.txt");
  std::string decrypted_password_file = std::string(TCOIN_PASS_PATH) + std::string(username) + std::string("_decrypted_password.txt");
  std::string password_file = std::string(TCOIN_PASS_PATH) + std::string(username) + std::string("_password.txt");

  std::ifstream fin(password_file.c_str());
  if(!fin || (fin && file_is_empty(fin)))
  {
    std::cout << "\nSorry, your password file could not be opened. You will have to create a new passphrase by running `tcoin init`.\n\n";
    return ERR_NO_PWD_FILE;
  }
  else
  {
    fin.close();

    std::ifstream codefin(TCOIN_CODEZ_PATH);
    char code1[513], code2[513], code3[513];
    codefin >> code1;
    codefin >> code2;
    codefin >> code3;
    codefin.close();

    exec2_5((std::string(TCOIN_BIN_PATH_W_SPACE) + std::string("code2")).c_str(), std::string(code2), 0); //0 because we don't want any output

    fin.open(decrypted_password_file.c_str());
    if(!fin || (fin && file_is_empty(fin)))
    {
      if(fin && file_is_empty(fin))
        remove(decrypted_password_file.c_str());
      fin.close();
      std::cout << "\nSorry, the passphrase you entered could not decrypt the encrypted password file. You are not logged on. Please run `tcoin on` to try again.\n\n";
      return ERR_WRONG_PWD;
    }
    else
    {
      if(files_are_same(salt_file.c_str(), decrypted_password_file.c_str()))
      {
        std::string salt_logged_in_file = std::string(TCOIN_SALT_PATH) + std::string(username) + std::string("_salt_logged_in.txt");
        rename(salt_file.c_str(), salt_logged_in_file.c_str());
        remove(decrypted_password_file.c_str());
        std::cout << "\nYou have now successfully logged on to tildecoin (please run `tcoin` to check your balance and messages). Please be aware that any programs you run now can siphon funds from your account without your knowing until it's too late. Please run `tcoin off` before running any untrusted programs.\n\n";
      }
      else
      {
        remove(decrypted_password_file.c_str());
        fin.close();
        std::cout << "\nSorry, the decrypted password file did not match the salt file. You are not logged on. Please run `tcoin on` to try again.\n\n";
        return ERR_WRONG_PWD2;
      }
    }
  }
  return 0;
}

int initialise_user(const char* username, const long long int &base_amount)
{
  std::string balance_file = std::string(TCOIN_PATH_W_SLASH) + std::string(username) + std::string(".txt");
  std::string messages_file = std::string(TCOIN_MSG_PATH) + std::string(username) + std::string("_messages.txt");
  std::string password_file = std::string(TCOIN_PASS_PATH) + std::string(username) + std::string("_password.txt");
  std::string password_candidate_file = std::string(TCOIN_PASS_PATH) + std::string(username) + std::string("_password_candidate.txt");
  std::string salt_file = std::string(TCOIN_SALT_PATH) + std::string(username) + std::string("_salt.txt");
  std::string salt_logged_in_file = std::string(TCOIN_SALT_PATH) + std::string(username) + std::string("_salt_logged_in.txt");

  std::ifstream fin(balance_file.c_str());
  bool flag_balance = false, flag_messages = false, flag_password_and_salt = false;
  if(!fin)
  {
    std::ofstream fout(balance_file.c_str(), std::fstream::trunc);
    fout << "0\n";
    fout.close();
    chmod(balance_file.c_str(), CHMOD_PERMISSIONS);
    flag_balance = true;
  }
  fin.close();

  fin.open(messages_file.c_str());
  if(!fin)
  {
    fin.close();
    std::ofstream fout(messages_file.c_str(), std::fstream::trunc);
    fout << "\n";
    fout.close();
    chmod(messages_file.c_str(), CHMOD_PERMISSIONS);
    flag_messages = true;
  }
  fin.close();

  fin.open(salt_file.c_str());
  std::ifstream fin2(password_file.c_str());
  std::ifstream fin3(salt_logged_in_file.c_str());
  std::ifstream fin4;
  if((!fin && !fin3) || !fin2 || (fin && file_is_empty(fin)) || (fin3 && file_is_empty(fin3))  || file_is_empty(fin2)) //if salt or password file is missing or empty, we'd have to set up a new salt and password (i.e., salt file encrypted with passphrase)
  {
    fin.close();
    fin2.close();
    fin3.close();

    remove(salt_file.c_str());
    remove(salt_logged_in_file.c_str());
    remove(password_file.c_str());

    std::ofstream fout(salt_file.c_str(), std::fstream::trunc);
    const long long int rand_max_length = strlen(std::to_string(RAND_MAX).c_str());
    {
      for(int i=0; i<16;++i)
      {
        fout.width(rand_max_length);
        fout.fill('0');
        fout << rand();
      }
    }
    fout << "\n";
    fout.close();
    chmod(salt_file.c_str(), CHMOD_PERMISSIONS);

    std::cout << "\nYour salt and/or password file(s) are missing. A new salt and password file will be created. Please enter your desired passphrase and re-enter to confirm the same below. You will need to enter it to log onto tildecoin. If you ^C before confirming the passphrase, you'll have created an empty password file and would have to run `tcoin init` again.\n\n";

    std::ifstream codefin(TCOIN_CODEZ_PATH);
    char code1[513], code2[513], code3[513];
    codefin >> code1;
    codefin >> code2;
    codefin >> code3;
    codefin.close();
    exec2_5((std::string(TCOIN_BIN_PATH_W_SPACE) + std::string("code1")).c_str(), std::string(code1), 0); //0 because we don't want any output

    //this file shouldn't exist, except if code1 was run by someone/some program other than tcoin,
    //in which case the password_file wouldn't have been removed, so we should fail if we detect
    //an already existing password_file
    fin4.open(password_file.c_str()); // this file shouldn't exist, so something has gone wrong if it does
    fin.open(password_candidate_file.c_str());
    if(!fin || (fin && file_is_empty(fin) || fin4))
    {
      if(file_is_empty(fin))
        chmod(password_candidate_file.c_str(), CHMOD_PERMISSIONS);
      fin.close();
      if(fin4) //password_file already exists, abort and don't replace
      {
        //just making sure the file has enough permissions to be deleted, since scrypt would have created it with different permissions
        chmod(password_candidate_file.c_str(), CHMOD_PERMISSIONS);
        remove(password_candidate_file.c_str());
      }
      fin4.close();
      std::cout << "\nSomething went wrong in the password-file generation process. Your password file is now empty. You will have to run `tcoin init` again and choose a new passphrase.\n\n";
      return 1;
    }
    else
    {
      fin.close();
      chmod(password_candidate_file.c_str(), CHMOD_PERMISSIONS);
      //rename password candidate file to password file to actualise the creation of the account
      while(1)
      {
        if(!std::rename(password_candidate_file.c_str(), password_file.c_str()))
          break;
      }
      chmod(password_file.c_str(), CHMOD_PERMISSIONS);
    }
    flag_password_and_salt=true;
  }
  fin.close();
  fin2.close();
  fin3.close();
  fin4.close();
  if(flag_balance==true)
  {
    std::cout << "\nWelcome to tildecoin. ";
    cout_formatted_amount(base_amount, " tildecoins have been added to your account.\n");
    if(flag_messages==false)
      std::cout << "\nPlease execute `tcoin --help` for help or just `tcoin` for a status update.\n";
  }
  if(flag_messages==true)
  {
    std::cout << "\nYour tildecoin account is ready to send and receive messages!";
    std::cout << "\nPlease execute `tcoin --help` for help or just `tcoin` for a status update.\n";
  }
  if(flag_password_and_salt==true)
  {
      std::cout << "\nThe password-file generation process was completed successfully. Please run `tcoin on` to log on and `tcoin off` to log off. While logged on, be aware that any other programs you run can siphon funds from your account without your knowing until it's too late. Thus, please run `tcoin off` before running any untrusted programs.\n";
  }
  if(!flag_balance && !flag_messages && !flag_password_and_salt) //the account was initialised despite being already initialised and having all the required files intact.
    std::cout << "\nYou took quite the chance initialising again. What if it nuked your balance, messages and passphrase?\n";
  std::cout << '\n';
  return 0;
}

int send_message(const char* sender_username, const char* receiver_username, const char* message, const long long int &amount_sent, const char* option)
{
  std::string random_string = std::to_string(rand());

  char *receiver_path = new char[strlen(receiver_username) + sizeof(TCOIN_MSG_PATH) + 13]; //sizeof() includes '\0'
  char *temp_receiver_path = new char[strlen(receiver_username) + strlen(random_string.c_str()) + sizeof(TCOIN_MSG_PATH) + 13];

  std::strcpy(receiver_path, TCOIN_MSG_PATH); //length = 27
  std::strcat(receiver_path, receiver_username);
  std::strcat(receiver_path, "_messages.txt"); //length = 13

  std::strcpy(temp_receiver_path, TCOIN_MSG_PATH);
  std::strcat(temp_receiver_path, receiver_username);
  std::strcat(temp_receiver_path, random_string.c_str());
  std::strcat(temp_receiver_path, "_messages.txt");

  //create receiver's message file if none exists
  //the message will be included in the receiver's
  //account when she/he initialises her/his account
  //at a later time

  char *receiver_salt_path = new char[strlen(receiver_username) + sizeof(TCOIN_SALT_PATH) + 9]; //sizeof() includes '\0'
  char *receiver_salt_logged_in_path = new char[strlen(receiver_username) + sizeof(TCOIN_SALT_PATH) + 19];
  std::strcpy(receiver_salt_path, TCOIN_SALT_PATH);
  std::strcat(receiver_salt_path, receiver_username);
  std::strcpy(receiver_salt_logged_in_path, receiver_salt_path);
  std::strcat(receiver_salt_path, "_salt.txt"); //length = 9
  std::strcat(receiver_salt_logged_in_path, "_salt_logged_in.txt"); //length = 19

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
      char *really_temp_receiver_path = new char[strlen(temp_receiver_path) + 5];  //4 + 1 for '\0'
      std::strcpy(really_temp_receiver_path, temp_receiver_path);
      std::strcat(really_temp_receiver_path, "_tmp"); //length = 4

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
        std::cerr << "Fatal error" << ERR_SEND_MESSAGE_RECEIVER_MSG_FILE_UNABLE_TO_BE_UPDATED_FATAL << ": the receiver message file was unable to be updated. Please contact " << TCOIN_ERR_CONTACT_EMAIL << " to report this error (because it requires manual recovery).";
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
              std::cerr << "Fatal error " << ERR_SEND_MESSAGE_PROGRAM_RECEIVER_MSG_FILE_UNABLE_TO_BE_UPDATED_FATAL << ": the receiver program_message file was unable to be updated. Please contact " << TCOIN_ERR_CONTACT_EMAIL << " to report this error (because it requires manual recovery).";
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

            // unlock_receiver_messages
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

      char *sender_path = new char[strlen(sender_username) + sizeof(TCOIN_MSG_PATH) + 13]; //sizeof() includes '\0'
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
          std::strcat(really_temp_sender_path, "_tmp"); // length = 4

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

bool user_is_blocked(const char* username)
{
  std::ifstream fin((std::string(TCOIN_PATH_W_SLASH) + std::string(username) + std::string("_blocked.txt")).c_str());
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

      // sizeof() includes the NULL character at the end of the string
      char* temp_sender_path = new char[strlen(sender_username) + strlen(random_string.c_str()) + sizeof(TCOIN_PATH_W_SLASH) + 4];
      char* sender_path = new char[strlen(sender_username) + sizeof(TCOIN_PATH_W_SLASH) + 4];
      char* temp_sender_username = new char[strlen(sender_username) + strlen(random_string.c_str()) + 1];

      std::strcpy(temp_sender_username, sender_username);
      std::strcat(temp_sender_username, random_string.c_str());

      std::strcpy(temp_sender_path, TCOIN_PATH_W_SLASH);
      std::strcat(temp_sender_path, temp_sender_username);
      std::strcat(temp_sender_path, ".txt");

      std::strcpy(sender_path, TCOIN_PATH_W_SLASH);
      std::strcat(sender_path, sender_username);
      std::strcat(sender_path, ".txt");

      while(1)
      {
        if(!std::rename(sender_path, temp_sender_path))
        {
          //Insufficient funds check is in add_file_value()
          //Returns 1 if insufficient funds, otherwise returns 0
          return_value = add_file_value(temp_sender_username, -1 * amount_to_send, base_amount);

          if(return_value == 0) // Funds sucessfully deducted from sender_username
          {
            random_string = std::to_string(rand());

            // sizeof() includes the NULL character at the end of the string
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
            char *receiver_salt_path = new char[strlen(receiver_username) + sizeof(TCOIN_SALT_PATH) + 9];
            // sizeof() includes the NULL character at the end of the string
            char *receiver_salt_logged_in_path = new char[strlen(receiver_username) + sizeof(TCOIN_SALT_PATH) + 19];
            std::strcpy(receiver_salt_path, TCOIN_SALT_PATH);
            std::strcat(receiver_salt_path, receiver_username);
            std::strcpy(receiver_salt_logged_in_path, receiver_salt_path);
            std::strcat(receiver_salt_path, "_salt.txt");
            std::strcat(receiver_salt_logged_in_path, "_salt_logged_in.txt");

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
                      }//value must also be added to a _total.txt file

                      while(1)
                      {
                        if(!std::rename(temp_program_receiver_path.c_str(), program_receiver_path.c_str()))
                          break;
                      }
                      break;
                    }//if statement with !std::rename for receiver's program accounting receiver balance file
                  }//while loop for program accounting receiver's balance file
                }//receiver is program account

                while(1)
                {
                  if(!std::rename(temp_receiver_path, receiver_path))
                    break;
                }
                delete[] temp_receiver_path;
                delete[] receiver_path;
                delete[] temp_receiver_username;
                break;
              }//if statement with !std::rename for receiver_username.txt
            }//while loop for receiver_username.txt

            if(!strcmp(option, "verbose"))
            {
              std::cout << "\n";
              cout_formatted_amount(amount_to_send, " tildecoins were ", " tildecoin was ");
              std::cout << "sent from `" << sender_username << "` to `" << receiver_username << "`.";
              std::cout << "\n\n";
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
        }//if statement with !std::rename
      }//while loop for sender
    }//amount sent is more than 0
  }//username exists
  else
  {
    if(!strcmp(option, "verbose"))
      std::cout << "\nSorry, no user with the username `" << receiver_username << "` was found.\n\n";
    return ERR_RECEIVER_NOT_FOUND;
  }

  return final_return_value;
}

void help(long long int &base_amount)
{
  std::cout << "\n - tildecoin (also called tcoin) is " << TCOIN_HOST_NAME << "'s very own digital (non-crypto) currency.";
  std::cout << "\n - to participate in " << TCOIN_HOST_NAME << "'s internal economy by creating your own tcoin account, run `tcoin init`.";
  std::cout << "\n - all users get "; cout_formatted_amount(base_amount); std::cout << " coins to start, and can send and receive coins from other users (and programs).";
  std::cout << "\n - more information about tildecoin is available at \"https://tilde.town/~login/tcoin.html\".\n";

  std::cout << "\n`tcoin`: check your balance and the 10 latest lines of messages";
  std::cout << "\n`tcoin on`: log on to tildecoin";
  std::cout << "\n`tcoin off`: out out of tildecoin";
  std::cout << "\n`tcoin init`: initialise tcoin (you'll need to do this before using anything else)";
  std::cout << "\n`tcoin balance` or `tcoin -b`: print the number representing your balance";
  std::cout << "\n`tcoin breakdown` or `tcoin -bd`: print a breakdown of how your balance was determined";
  std::cout << "\n`tcoin messages` or `tcoin -m`: print all messages";
  std::cout << "\n`tcoin messages <num>` or `tcoin -m <num>`: print the last <num> messages";
  std::cout << "\n`tcoin clear_messages` or `tcoin -cm`: clear all messages";
  std::cout << "\n`tcoin send <username> <amount>` or `tcoin -s <username> <amount>`: send <amount> tildecoins to <username>";
  std::cout << "\n`tcoin send <username> <amount> \"<message>\"` or `tcoin -s <username> <amount> \"<message>\"`: optionally, include a message to be sent to <username>";
  std::cout << "\n`tcoin silentsend <username> <amount> [\"<message>\"]`, `tcoin send -s <username> <amount> [\"<message>\"]` or `tcoin -ss <username> <amount> [\"<message>\"]`: send <amount> tildecoins to <username> with an optional (as indicated by [ and ], which should not be included in the actual comment) message included without printing anything";
  std::cout << "\nIn the commands with `<username> <amount>`, switching the two arguments around (i.e., from `<username> <amount>` to `<amount> <username>`) will also work";
  std::cout << "\n`tcoin --help`, `tcoin help` or `tcoin -h`: print this help text";
  std::cout << "\nSend an email to " << TCOIN_PASS_RESET_CONTACT_EMAIL << " to request a passphrase reset.\n\n";
}

bool is_number(const char* test_string)
{
    char* p;
    std::strtod(test_string, &p);
    return *p == 0;
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
  //quick exit if user is blocked
  if(user_is_blocked(get_username().c_str()))
  {
    return ERR_USER_IS_BLOCKED;
  }
  //sneaky scrypt magic (process overlaying to maintain suid)
  {
    std::ifstream codefin(TCOIN_CODEZ_PATH);
    char code1[513], code2[513], code3[513], codecheck[514];
    codefin >> code1;
    codefin >> code2;
    codefin >> code3;
    codefin.close();
    if(argc==2 && !strctcmp(argv[1], "code1"))
    {
      //to set the password when doing 'tcoin init', we create a hashed + encrypted <username>_password_candidate.txt
      //the code that calls this will then check if <username>_password.txt doesn't already exist, and if it doesn't,
      //it'll move <username>_password_candidate.txt to <username>_password.txt, otherwise it'll delete
      //<username>_password_candidate.txt
      std::fgets(codecheck, 514, stdin);
      codecheck[513] = codecheck[514] = '\0';
      if(!strctcmp(codecheck, code1))
      {
        std::string salt_file = std::string(TCOIN_SALT_PATH) + get_username() + std::string("_salt.txt");
        std::string password_file = std::string(TCOIN_PASS_PATH) + get_username() + std::string("_password_candidate.txt");
        execl(TCOIN_SCRYPT_PATH, "scrypt", "enc", "-m", "0.125", "-t", "5", salt_file.c_str(), password_file.c_str(), NULL);
      }
    }
    if(argc==2 && !strctcmp(argv[1], "code2"))
    {
      std::fgets(codecheck, 514, stdin);
      codecheck[513] = codecheck[514] = '\0';
      if(!strctcmp(codecheck, code2))
      {
        std::string decrypted_password_file = std::string(TCOIN_PASS_PATH) + get_username() + std::string("_decrypted_password.txt");
        std::string password_file = std::string(TCOIN_PASS_PATH) + get_username() + std::string("_password.txt");
        execl(TCOIN_SCRYPT_PATH, "scrypt", "dec", "-t", "15", password_file.c_str(), decrypted_password_file.c_str(), NULL);
      }
    }
    if(argc==2 && !strctcmp(argv[1], "pcoin_list"))
    {
      std::fgets(codecheck, 514, stdin);
      codecheck[513] = codecheck[514] = '\0';
      if(!strctcmp(codecheck, code3))
        execl(LS_PATH, "ls", PCOIN_KEY_PATH, NULL);
    }
  }
  //If ^C is sent while doing `tcoin on`, <username>_dercrypted_password.txt gets left behind
  //this might cause the program to interpret the salt and password to be corrupted, and might
  //ask to create a new passphrase. To prevent this, we cleanup _decrypted_password.txt on every
  //start of tcoin. We do the same for _password_candidate.txt, which would be left over if someone
  //were to inadvertently find code1 and use that to try to replace the password without having logged in
  {
    std::string decrypted_password_file = std::string(TCOIN_PASS_PATH) + get_username() + std::string("_decrypted_password.txt");
    remove(decrypted_password_file.c_str());

    std::string password_candidate_file = std::string(TCOIN_PASS_PATH) + get_username() + std::string("_password_candidate.txt");
    remove(decrypted_password_file.c_str());
  }

  base_amount = get_file_value("base/base");
  long long int unaltered_base_amount = base_amount;

  //adding tildebot scores from krowbar to base amount
  #ifndef KROWBAR_OFF
  if(!user_is_locked(get_username().c_str()))
  {
    std::string line;
    const std::string username = get_username();
    const int username_length = username.length();
    std::string score_file_path;
    std::ifstream fin;

    for(int j=0; j<2; ++j)
    {
      if(j == 0)
        score_file_path.assign(KROWBAR_SCORE_PATH);
      else if(j == 1)
        score_file_path.assign(JU_SCORE_PATH);

      fin.open(score_file_path);

      while(std::getline(fin, line))
      {
        char* line_c_string = new char[line.length()+1];
        std::strcpy(line_c_string, line.c_str());

        const int irc_username_length = username_length > USERNAME_LENGTH_LIMIT ? USERNAME_LENGTH_LIMIT : username_length;

        if(!std::strncmp(username.c_str(), line_c_string, irc_username_length))
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

          krowbar_amount[j] = strtol100(number_of_tildes);

          base_amount += krowbar_amount[j];
          //multiplied by 100 inside strtol100() to convert tildecoins to centitildecoins, which
          //is the unit used throughout the program (and converted appropriately when displayed)
          break;
        }
        delete[] line_c_string;
      }
      fin.close();
    }
  }
  #endif

  #ifndef DA_OFF
  //adding daily-adventure scores from troido to base amount
  if(!user_is_locked(get_username().c_str()))
  {
    std::string number_of_tildes = exec(TROIDO_DACOINS_CMD);
    number_of_tildes.pop_back();
    //to get rid of the newline at the end
    if(is_number(number_of_tildes.c_str()))
      da_amount += strtol100(number_of_tildes.c_str());
      base_amount += da_amount;
      //multiplied by 100 to convert tildecoins to centitildecoins, which
      //is the unit used throughout the program (and converted appropriately when displayed)
  }
  #endif

  #ifndef MINERCOIN_OFF
  if(!user_is_locked(get_username().c_str()))
  {
    std::string minercoin_cmd_pre_username, command_to_exec, number_of_tildes, minercoin_cmd_post_username(MINERCOIN_CMD_POST_USERNAME);

    //adding minercoin scores from minerobber to base amount (from "~username" and "username" entries in minerbot)
    for(int i=0; i < 2; ++i)
    {
      if(i == 0)
        minercoin_cmd_pre_username.assign(MINERCOIN_CMD_PRE_USERNAME);
      else if(i == 1)
        minercoin_cmd_pre_username.assign(MINERCOIN_CMD_PRE_USERNAME2);

      command_to_exec = minercoin_cmd_pre_username + get_username() + minercoin_cmd_post_username;
      number_of_tildes = exec(command_to_exec.c_str());
      number_of_tildes.pop_back();
      //to get rid of the newline at the end
      if(is_number(number_of_tildes.c_str()))
      {
        minercoin_amount[i] = strtol100(number_of_tildes.c_str());
        base_amount += minercoin_amount[i];
        //multiplied by 100 to convert tildecoins to centitildecoins, which
        //is the unit used throughout the program (and converted appropriately when displayed)
      }
    }
  }
  #endif
  srand((long int)(std::time(NULL)) + strtol_fast(exec(BIN_ECHO_CMD).c_str()));

  if(argc > 1 && (!strcmp(argv[1], "--help") || !strcmp(argv[1], "help") || !strcmp(argv[1], "-h")))
  {
    help(unaltered_base_amount);
    return 0;
  }
  if(argc > 1 && !strcmp(argv[1], "init"))
  {
    initialise_user(get_username().c_str(), base_amount);
    return 0;
  }
  if(!user_has_initialised(get_username().c_str()))
  {
    std::cout << "\nSorry, tcoin has not been initialised. Please execute `tcoin init` to complete initialisation or `tcoin help` for help.\n\n";
    return ERR_NO_INIT;
  }
  if(argc > 1 && !strcmp(argv[1], "on"))
  {
    return log_on(get_username().c_str()); //return codes are inside the log_on function
  }
  if(argc > 1 && !strcmp(argv[1], "off"))
  {
    return log_off(get_username().c_str()); //return codes are inside the log_off function
  }
  if(!is_logged_on(get_username().c_str()))
  {
    std::cout << "\nSorry, you have not logged on to tildecoin yet. Please execute `tcoin on` to log in or `tcoin help` for help.\n\n";
    return ERR_NOT_LOGGED_IN;
  }

  user_amount = get_file_value(get_username().c_str());
  long long int total_amount = base_amount + user_amount;

  if(argc < 2)
  {
    //show last 10 messages
    show_messages_tail(get_username().c_str(), 10);
    show_balance(get_username().c_str(), total_amount, unaltered_base_amount, "base amount", user_amount, "transfers", krowbar_amount[0], "tilde game", krowbar_amount[1], "ju game", da_amount, "daily-adventure game", minercoin_amount[0]+minercoin_amount[1], "MinerCoin game");
  }
  else if(!strcmp(argv[1], "breakdown") || !strcmp(argv[1], "-bd"))
  {
    std::cout << "Total balance: ";
    cout_formatted_amount(total_amount, " tildecoins\n", " tildecoin\n");
    show_breakdown(unaltered_base_amount, "base amount", user_amount, "transfers", krowbar_amount[0], "tilde game", krowbar_amount[1], "ju game", da_amount, "daily-adventure game", minercoin_amount[0]+minercoin_amount[1], "MinerCoin game");
  }
  else if(!strcmp(argv[1], "balance") || !strcmp(argv[1], "-b"))
    cout_formatted_amount(total_amount, "\n");
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
  else if(!strcmp(argv[1], "clear_messages") || !strcmp(argv[1], "-cm"))
  {
    clear_messages(get_username().c_str());
  }
  else if(!strcmp(argv[1], "send") || !strcmp(argv[1], "-s"))
  {
    if(argc == 5)
    {
      if(!strcmp(argv[2], "-s")) //silent send with no message
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
        char *receiver = NULL;

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
      std::cout << "\nSorry, too few command-line arguments were passed. The correct format is `tcoin send <username> <amount>`.\n\n";
      return ERR_MAIN_SEND_TOO_FEW_ARGS;
    }
    else if(argc > 4)
    {
      std::cout << "\nSorry, too many command-line arguments were passed. The correct format is `tcoin send <username> <amount>`.\n\n";
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
    else if(argc==5) //custom message included
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
  else
  {
    std::cout << "\nSorry, an unknown command-line argument was received. `tcoin help` will print the help text.\n\n";
    return ERR_UNKNOWN_ARG;
  }

  return 0;
}
