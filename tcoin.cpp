#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <sstream>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <array>
#include <string>
#include <algorithm>
#include <iterator>
#include <sys/stat.h>
#include <ctime>
#include <unistd.h>

void exit_program(const int error_number)
{
  // Cleanup to do before exiting the program

  // Finally, we can exit
  std::exit(error_number);
}

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
            result += buffer.data();
    }
    return result;
}

long long int get_file_value(const char* file_name)
{
  char* file_path = new char[strlen(file_name)+23];
  std::strcpy(file_path, "/home/login/tcoin/");
  std::strcat(file_path, file_name);
  std::strcat(file_path, ".txt");

  std::fstream file(file_path);

  if(!file)
  {
    if(!strcmp(file_name, "base/base"))
    {
      std::cerr << "\nError! Could not open file at " << file_path << "!\n\n";
      exit_program(1);
    }
  }

  std::ostringstream ss;
  ss << file.rdbuf();

  delete[] file_path;
  return std::strtol(ss.str().c_str(), NULL, 10);
}

int add_file_value(const char* file_name, const long long int &value_to_add, const long long int &base_amount)
{
  char* file_path = new char[strlen(file_name)+23];
  std::strcpy(file_path, "/home/login/tcoin/");
  std::strcat(file_path, file_name);
  std::strcat(file_path, ".txt");

  std::fstream file(file_path);

  if(!file)
  {
    if(!strcmp(file_name, "base/base"))
    {
      std::cerr << "\nError! Could not open file at " << file_path << "!\n\n";
      file.close();
      delete[] file_path;
      exit_program(1);
    }
  }

  std::ostringstream ss;
  ss << file.rdbuf();

  long long int old_value = std::strtol(ss.str().c_str(), NULL, 10);

  //sufficient funds check
  if(value_to_add < 0 && (old_value + base_amount + value_to_add < 0))
  {
    file.close();
    delete[] file_path;
    return 1;
  }

  long long int new_value = old_value + value_to_add;

  // Writing new value to file
  file.close();
  file.open(file_path, std::fstream::out | std::fstream::trunc);
  file << new_value << "\n";
  file.close();

  delete[] file_path;
  return 0;
}

std::string get_username()
{
  static std::string username;
  if(username.empty())
  {
    username = exec("/usr/bin/whoami");
    username.pop_back(); //to get rid of newline at the end
  }
  return username;
}

long long int base_amount;
long long int user_amount;

void show_balance(const char* username, const long long int &amount)
{
  if(amount == 100)
    std::cout << "\n" << username << ", you have " << amount/100 << " tildecoin to your name.";
  else if(amount % 100 == 0)
    std::cout << "\n" << username << ", you have " << amount/100 << " tildecoins to your name.";
  else if(amount % 100 < 10)
    std::cout << "\n" << username << ", you have " << amount/100 << ".0" << amount % 100 <<" tildecoins to your name.";
  else
    std::cout << "\n" << username << ", you have " << amount/100 << "." << amount % 100 <<" tildecoins to your name.";

  std::cout << "\nThe command to send tildecoins to other users is `tcoin send <username> <amount>` or `tcoin -s <username> <amount>`";
  std::cout << "\nThe command to log out of tildecoin is `tcoin off`.\n\n";
}

void show_messages(const char* username)
{
  std::string messages_path = std::string("/home/login/tcoin/messages/") + std::string(username) + std::string("_messages.txt");
  std::ifstream fin(messages_path.c_str());
  std::cout << "Messages:\n\n";
  std::cout << fin.rdbuf();
  std::cout << "\n";
  fin.close();
}

bool username_exists(const char* username)
{
  const static std::string all_usernames = exec("/bin/ls /home");
  std::istringstream iss(all_usernames);
  static std::vector<std::string> usernames{std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{}};
  if(std::find(usernames.begin(), usernames.end(), username) != usernames.end())
  {
    return true;
  }
  return false;
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
  fin2.get(c2); //when fin1.get(c1) is faled, fin2.get(c2) is not executed because of short-circuitted boolean operators. This line compensates for that.
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

bool user_has_initialised(const char* username)
{
  bool return_value = true; //we assume the user has initialised and check for signs of that not being the case

  char *balance_file_path = new char[strlen(username) + 23];
  std::strcpy(balance_file_path, "/home/login/tcoin/");
  std::strcat(balance_file_path, username);
  std::strcat(balance_file_path, ".txt");

  char *messages_file_path = new char[strlen(username) + 41];
  std::strcpy(messages_file_path, "/home/login/tcoin/messages/");
  std::strcat(messages_file_path, username);
  std::strcat(messages_file_path, "_messages.txt");

  char *password_file_path = new char[strlen(username) + 42];
  std::strcpy(password_file_path, "/home/login/tcoin/passwords/");
  std:;strcat(password_file_path, username);
  std::strcat(password_file_path, "_password.txt");

  char *salt_file_path = new char[strlen(username) + 34];
  std::strcpy(salt_file_path, "/home/login/tcoin/salts/");
  std::strcat(salt_file_path, username);
  std::strcat(salt_file_path, "_salt.txt");

  char *salt_logged_in_file_path = new char[strlen(username) + 44];
  std::strcpy(salt_logged_in_file_path, "/home/login/tcoin/salts/");
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
  std::string salt_logged_in_file = std::string("/home/login/tcoin/salts/") + std::string(username) + std::string("_salt_logged_in.txt");
  std::string salt_file = std::string("/home/login/tcoin/salts/") + std::string(username) + std::string("_salt.txt");
  std::ifstream fin(salt_logged_in_file.c_str());
  if(!fin) //user is not currently logged in
  {
    std::cout << "\nSorry, you're already logged out. Thanks for being extra careful about being logged out before running untrusted programs (if you were going to do that).\n\n";
    return 1;
  }
  else //user is logged in
  {
    if(file_is_empty(fin)) //user is logged in but salt_logged_in_file is empty
    {
      remove(salt_logged_in_file.c_str());
      remove(salt_file.c_str());
      std::cout << "\nSorry, your salt file is damaged. You will have to run `tcoin init` and create a new passphrase.\n\n";
      return 2;
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
        return 3;
      }
    }
  }
  return 0;
}

bool is_logged_on(const char* username)
{
  std::string salt_logged_in_file = std::string("/home/login/tcoin/salts/") + std::string(username) + std::string("_salt_logged_in.txt");
  std::string salt_file = std::string("/home/login/tcoin/salts/") + std::string(username) + std::string("_salt.txt");
  std::ifstream fin(salt_logged_in_file.c_str());
  if(!fin)
    return false;
  else if(file_is_empty(fin))
  {
      fin.close();
      remove(salt_logged_in_file.c_str());
      remove(salt_file.c_str());
      std::cout << "\n\nYour salt logged in file is empty. You'll have to run `tcoin init` and creata a new passphrase.\n\n";
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
    return 4;
  }
  std::string salt_file = std::string("/home/login/tcoin/salts/") + std::string(username) + std::string("_salt.txt");
  std::string decrypted_password_file = std::string("/home/login/tcoin/passwords/") + std::string(username) + std::string("_decrypted_password.txt");
  std::string password_file = std::string("/home/login/tcoin/passwords/") + std::string(username) + std::string("_password.txt");

  std::ifstream fin(password_file.c_str());
  if(!fin || (fin && file_is_empty(fin)))
  {
    std::cout << "\nSorry, your password file could not be opened. You will have to create a new passphrase by running `tcoin init`.\n\n";
    return 1;
  }
  else
  {
    fin.close();

    std::ifstream codefin("/home/login/bin/tcoin_codez");
    char code1[17], code2[17];
    codefin >> code1;
    codefin >> code2;
    codefin.close();
    exec((std::string("/home/login/bin/tcoin ") + std::string(code2)).c_str());

    fin.open(decrypted_password_file.c_str());
    if(!fin || (fin && file_is_empty(fin)))
    {
      if(fin && file_is_empty(fin))
        remove(decrypted_password_file.c_str());
      fin.close();
      std::cout << "\nSorry, the passphrase you entered could not decrypt the encrypted password file. You are not logged on. Please run `tcoin on` to try again.\n\n";
      return 2;
    }
    else
    {
      if(files_are_same(salt_file.c_str(), decrypted_password_file.c_str()))
      {
        std::string salt_logged_in_file = std::string("/home/login/tcoin/salts/") + std::string(username) + std::string("_salt_logged_in.txt");
        rename(salt_file.c_str(), salt_logged_in_file.c_str());
        remove(decrypted_password_file.c_str());
        std::cout << "\nYou have now successfully logged on to tildecoin. Please be aware that any programs you run now can siphon funds from your account without your knowing until it's too late. Please run `tcoin off` before running any untrusted programs.\n\n";
      }
      else
      {
        remove(decrypted_password_file.c_str());
        fin.close();
        std::cout << "\nSorry, the decrypted password file did not match the salt file. You are not logged on. Please run `tcoin on` to try again.\n\n";
        return 3;
      }
    }
  }
  return 0;
}

int initialise_user(const char* username, const long long int &base_amount)
{
  std::string balance_file = std::string("/home/login/tcoin/") + std::string(username) + std::string(".txt");
  std::string messages_file = std::string("/home/login/tcoin/messages/") + std::string(username) + std::string("_messages.txt");
  std::string password_file = std::string("/home/login/tcoin/passwords/") + std::string(username) + std::string("_password.txt");
  std::string salt_file = std::string("/home/login/tcoin/salts/") + std::string(username) + std::string("_salt.txt");
  std::string salt_logged_in_file = std::string("/home/login/tcoin/salts/") + std::string(username) + std::string("_salt_logged_in.txt");

  std::ifstream fin(balance_file.c_str());
  bool flag_balance = false, flag_messages = false, flag_password_and_salt = false;
  if(!fin)
  {
    std::ofstream fout(balance_file.c_str(), std::fstream::trunc);
    fout << "0\n";
    fout.close();
    chmod(balance_file.c_str(), (S_IRUSR | S_IWUSR) & ~S_IRWXG & ~S_IRWXO);
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
    chmod(messages_file.c_str(), (S_IRUSR | S_IWUSR) & ~S_IRWXG & ~S_IRWXO);
    flag_messages = true;
  }
  fin.close();

  fin.open(salt_file.c_str());
  std::ifstream fin2(password_file.c_str());
  std::ifstream fin3(salt_logged_in_file.c_str());
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
    chmod(salt_file.c_str(), (S_IRUSR | S_IWUSR) & ~S_IRWXG & ~S_IRWXO);

    std::cout << "\nYour salt and/or password file(s) are missing. A new salt and password file will be created. Please enter your desired passphrase and re-enter to confirm the same below. You will need to enter it to log onto tildecoin. If you ^C before confirming the passphrase, you'll have created an empty password file and would have to run `tcoin init` again.\n\n";

    std::ifstream codefin("/home/login/bin/tcoin_codez");
    char code1[17], code2[17];
    codefin >> code1;
    codefin.close();
    exec((std::string("/home/login/bin/tcoin ") + std::string(code1)).c_str());

    fin.open(password_file.c_str());
    if(!fin || (fin && file_is_empty(fin)))
    {
      if(file_is_empty(fin))
        chmod(password_file.c_str(), (S_IRUSR | S_IWUSR) & ~S_IRWXG & ~S_IRWXO);
      fin.close();
      std::cout << "\nSomething went wrong in the password-file generation process. Your password file is now empty. You will have to run `tcoin init` again and choose a new passphrase.\n\n";
      return 1;
    }
    else
    {
      chmod(password_file.c_str(), (S_IRUSR | S_IWUSR) & ~S_IRWXG & ~S_IRWXO);
      fin.close();
    }
  flag_password_and_salt=true;
  }
  fin.close();
  fin2.close();
  fin3.close();
  if(flag_balance==true)
  {
    if(base_amount % 100 == 0)
      std::cout << "\nWelcome to tildecoin. " << base_amount/100 << " tildecoins have been added to your account.";
    else if(base_amount % 100 < 10)
      std::cout << "\nWelcome to tildecoin. " << base_amount/100 << ".0" << base_amount % 100  << " tildecoins have been added to your account.";
    else
      std::cout << "\nWelcome to tildecoin. " << base_amount/100 << "." << base_amount % 100 << " tildecoins have been added to your account.";
    if(flag_messages==false)
      std::cout << "\nPlease execute `tcoin --help` for help or just `tcoin` for a status update.\n\n";
  }
  if(flag_messages==true)
  {
    std::cout << "\nYour tildecoin account is ready to send and receive messages!";
    std::cout << "\nPlease execute `tcoin --help` for help or just `tcoin` for a status update.\n\n";
  }
  if(flag_password_and_salt==true)
  {
      std::cout << "\nThe password-file generation process was completed successfully. Please run `tcoin on` to log on and `tcoin off` to log off. While logged on, be aware that any other programs you run can siphon funds from your account without your knowing until it's too late. Thus, please run `tcoin off` before running any untrusted programs.\n\n";
  }
  if(!flag_balance && !flag_messages && !flag_password_and_salt) //the account was initialised despite being already initialised and having all the required files intact.
    std::cout << "\nYou took quite the chance initialising again. What if it nuked your balance, messages and passphrase?\n\n";
  return 0;
}

int send_message(const char* sender_username, const char* receiver_username, const char* message, const long long int &amount_sent)
{
  std::string random_string = std::to_string(rand());

  char *receiver_path = new char[strlen(receiver_username) + 41];
  char *temp_receiver_path = new char[strlen(receiver_username) + strlen(random_string.c_str()) + 41];

  std::strcpy(receiver_path, "/home/login/tcoin/messages/");
  std::strcat(receiver_path, receiver_username);
  std::strcat(receiver_path, "_messages.txt");

  std::strcpy(temp_receiver_path, "/home/login/tcoin/messages/");
  std::strcat(temp_receiver_path, receiver_username);
  std::strcat(temp_receiver_path, random_string.c_str());
  std::strcat(temp_receiver_path, "_messages.txt");

  //create receiver's message file if none exists
  //the message will be included in the receiver's
  //account when she/he initialises her/his account
  //at a later time
  std::ifstream fin(receiver_path);
  if(!fin || file_is_empty(fin))
  {
    fin.close();
    std::ofstream fout(receiver_path, std::fstream::trunc);
    fout << "\n";
    fout.close();
  }
  fin.close();
  chmod(receiver_path, (S_IRUSR | S_IWUSR) & ~S_IRWXG & ~S_IRWXO);


  while(1)
  {
    if(std::rename(receiver_path, temp_receiver_path))
    {
      char *really_temp_receiver_path = new char[strlen(temp_receiver_path) + 4];
      std::strcpy(really_temp_receiver_path, temp_receiver_path);
      std::strcat(really_temp_receiver_path, "_tmp");


      chmod(really_temp_receiver_path, (S_IRUSR | S_IWUSR) & ~S_IRWXG & ~S_IRWXO);
      std::ifstream fin(temp_receiver_path);
      std::ofstream fout(really_temp_receiver_path);

      if(!file_is_empty(fin))
        fout << fin.rdbuf();
      fin.close();

      time_t now = time(0);
      char* dt = std::ctime(&now);
      dt[strlen(dt)-1]='\0';
      char sender_formatted_string[26];
      char sender_arrow_formatted_string[47];
      char sender_arrow_string[37];
      char receiver_formatted_string[26];
      std::snprintf(sender_formatted_string, 26, "%25s", sender_username);
      std::snprintf(receiver_formatted_string, 26, "%-25s", receiver_username);
      int sender_username_length = std::strlen(sender_username);
      int number_of_chars = 26 >= sender_username_length ? 26 : sender_username_length;
      std::strncpy(sender_arrow_string, sender_username, number_of_chars);
      sender_arrow_string[number_of_chars] = '\0';
      std::strcat(sender_arrow_string, " ----");
      if(amount_sent % 100 == 0)
      {
        std::strncat(sender_arrow_string, std::to_string(amount_sent/100).c_str(), 10);
      }
      else if(amount_sent % 100 < 10)
      {
        std::strncat(sender_arrow_string, std::to_string(amount_sent/100).c_str(), 10);
        std::strncat(sender_arrow_string, ".0", 10);
        std::strncat(sender_arrow_string, std::to_string(amount_sent % 100).c_str(), 10);
      }
      else
      {
        std::strncat(sender_arrow_string, std::to_string(amount_sent/100).c_str(), 10);
        std::strncat(sender_arrow_string, ".", 10);
        std::strncat(sender_arrow_string, std::to_string(amount_sent % 100).c_str(), 10);
      }
      std::strcat(sender_arrow_string, "----> ");
      std::snprintf(sender_arrow_formatted_string, 47, "%46s", sender_arrow_string);
      fout << dt << ": " << sender_arrow_formatted_string << receiver_username;
      if(!strcmp(message, "")) //if message is empty
        fout << "\n";
      else
        fout << "\n \\_ " << sender_username << " said: " << message << "\n\n";

      fout.close();

      std::remove(temp_receiver_path);
      std::rename(really_temp_receiver_path, temp_receiver_path);

      std::rename(temp_receiver_path, receiver_path);

      chmod(receiver_path, (S_IRUSR | S_IWUSR) & ~S_IRWXG & ~S_IRWXO);

      delete[] really_temp_receiver_path;
      delete[] temp_receiver_path;
      delete[] receiver_path;

      //locking sender_messages_while_receiver_messages_locked

      random_string = std::to_string(rand());

      char *sender_path = new char[strlen(sender_username) + 41];
      char *temp_sender_path = new char[strlen(sender_username) + strlen(random_string.c_str()) + 41];

      std::strcpy(sender_path, "/home/login/tcoin/messages/");
      std::strcat(sender_path, sender_username);
      std::strcat(sender_path, "_messages.txt");

      std:strcpy(temp_sender_path, "/home/login/tcoin/messages/");
      std::strcat(temp_sender_path, sender_username);
      std::strcat(temp_sender_path, random_string.c_str());
      std::strcat(temp_sender_path, "_messages.txt");

      while(1)
      {
        if(std::rename(sender_path, temp_sender_path))
        {
          char *really_temp_sender_path = new char[strlen(temp_sender_path) + 4];
          std::strcpy(really_temp_sender_path, temp_sender_path);
          std::strcat(really_temp_sender_path, "_tmp");

          fin.open(temp_sender_path);
          fout.open(really_temp_sender_path);
          chmod(really_temp_sender_path, (S_IRUSR | S_IWUSR) & ~S_IRWXG & ~S_IRWXO);

          fout << fin.rdbuf();

          now = time(0);
          dt = std::ctime(&now);
          dt[strlen(dt)-1]='\0';
          char sender_formatted_string_right_aligned[26];
          char receiver_arrow_formatted_string[47];
          char receiver_arrow_string[37];
          std::snprintf(receiver_formatted_string, 26, "%25s", receiver_username);
          std::snprintf(sender_formatted_string, 26, "%-25s", sender_username);
          std::snprintf(sender_formatted_string_right_aligned, 26, "%25s", sender_username);
          int receiver_username_length = std::strlen(receiver_username);
          int number_of_chars = 26 >= receiver_username_length ? 26 : receiver_username_length;
          std::strncpy(receiver_arrow_string, receiver_username, number_of_chars);
          receiver_arrow_string[number_of_chars] = '\0';
          std::strcat(receiver_arrow_string, " <---");
          if(amount_sent % 100 == 0)
          {
            std::strncat(receiver_arrow_string, std::to_string(amount_sent/100).c_str(), 10);
          }
          else if(amount_sent % 100 < 10)
          {
            std::strncat(receiver_arrow_string, std::to_string(amount_sent/100).c_str(), 10);
            std::strncat(receiver_arrow_string, ".0",10);
            std::strncat(receiver_arrow_string, std::to_string(amount_sent % 100).c_str(), 10);
          }
          else
          {
            std::strncat(receiver_arrow_string, std::to_string(amount_sent/100).c_str(), 10);
            std::strncat(receiver_arrow_string, ".", 10);
            std::strncat(receiver_arrow_string, std::to_string(amount_sent % 100).c_str(), 10);
          }
          std::strcat(receiver_arrow_string, "----- ");
          std::snprintf(receiver_arrow_formatted_string, 47, "%46s", receiver_arrow_string);
          fout << dt << ": " << receiver_arrow_formatted_string << sender_username;
          if(!strcmp(message, "")) //if message is empty
            fout << "\n";
          else
            fout << "\n \\_ " << sender_username << " said: " << message << "\n\n";

          fin.close();
          fout.close();

          std::remove(temp_sender_path);
          std::rename(really_temp_sender_path, temp_sender_path);

          std::rename(temp_sender_path, sender_path);

          delete[] really_temp_sender_path;
          delete[] temp_sender_path;
          delete[] sender_path;
          break;
        }
      }
      break;
    }
  }

  return 0;
}

bool user_is_locked(const char* username)
{
  std::ifstream fin((std::string("/home/login/tcoin/") + std::string(username) + std::string("_locked.txt")).c_str());
  if(!fin)
    return false;
  return true;
}

int send(const char* sender_username, const char* receiver_username, const long long int &amount_to_send, const long long int &base_amount, const char* option)
{
  //receiver usrname check
  if(username_exists(receiver_username))
  {
    if(user_is_locked(receiver_username))
    {
      if(!strcmp(option, "verbose"))
        std::cout << "\nSorry, `" << receiver_username << "` does not wish to receive any tildecoins at this time.\n\n";
      return 4;
    }
    if(amount_to_send <= 0)
    {
      if(!strcmp(option, "verbose"))
        std::cout << "\nSorry, that amount is not valid. The amount should be a positive decimal number when truncated to two decimal places.\n\n";
        return 2;
    }
    else
    {
      std::string random_string = std::to_string(rand());
      int return_value = -1;

      char* temp_sender_path = new char[strlen(sender_username) + strlen(random_string.c_str()) + 23];
      char* sender_path = new char[strlen(sender_username) + 23];
      char* temp_sender_username = new char[strlen(sender_username) + strlen(random_string.c_str()) + 1];

      std::strcpy(temp_sender_username, sender_username);
      std::strcat(temp_sender_username, random_string.c_str());

      std::strcpy(temp_sender_path, "/home/login/tcoin/");
      std::strcat(temp_sender_path, temp_sender_username);
      std::strcat(temp_sender_path, ".txt");

      std::strcpy(sender_path, "/home/login/tcoin/");
      std::strcat(sender_path, sender_username);
      std::strcat(sender_path, ".txt");

      while(1)
      {
        if(std::rename(sender_path, temp_sender_path))
        {
          //Insufficient funds check is in add_file_value()
          return_value = add_file_value(temp_sender_username, -1 * amount_to_send, base_amount);
          std::rename(temp_sender_path, sender_path);
          delete[] temp_sender_path;
          delete[] sender_path;
          delete[] temp_sender_username;
          break;
        }
      }

      if(return_value == 0) // Funds sucessfully deducted from sender_username
      {
        random_string = std::to_string(rand());

        char *temp_receiver_path = new char[strlen(receiver_username) + strlen(random_string.c_str()) + 23];
        char *receiver_path = new char[strlen(receiver_username) + 23];
        char *temp_receiver_username = new char[strlen(receiver_username) + strlen(random_string.c_str()) + 1];

        std::strcpy(temp_receiver_username, receiver_username);
        std::strcat(temp_receiver_username, random_string.c_str());

        std::strcpy(temp_receiver_path, "/home/login/tcoin/");
        std::strcat(temp_receiver_path, temp_receiver_username);
        std::strcat(temp_receiver_path, ".txt");

        std::strcpy(receiver_path, "/home/login/tcoin/");
        std::strcat(receiver_path, receiver_username);
        std::strcat(receiver_path, ".txt");

        //create receiver's balance file if none exists
        //the balance will be included in the receiver's
        //account when she/he initialises her/his account
        //at a later time
        std::ifstream fin(receiver_path);
        if(!fin || file_is_empty(fin))
        {
          fin.close();
          std::ofstream fout(receiver_path, std::fstream::trunc);
          fout << "\n";
          fout.close();
        }
        fin.close();
        chmod(receiver_path, (S_IRUSR | S_IWUSR) & ~S_IRWXG & ~S_IRWXO);

        while(1)
        {
          if(std::rename(receiver_path, temp_receiver_path))
          {
            //Insufficient funds check in add_file_value() itself
            add_file_value(temp_receiver_username, amount_to_send, base_amount);
            std::rename(temp_receiver_path, receiver_path);
            delete[] temp_receiver_path;
            delete[] receiver_path;
            delete[] temp_receiver_username;
            break;
          }
        }

        if(!strcmp(option, "verbose"))
        {
          if(amount_to_send == 100)
            std::cout << "\n" <<  amount_to_send/100 << " tildecoin was sent from `" << sender_username << "` to `" << receiver_username << "`.";
          else if(amount_to_send % 100 == 0)
            std::cout << "\n" <<  amount_to_send/100 << " tildecoins were sent from `" << sender_username << "` to `" << receiver_username << "`.";
          else if(amount_to_send % 100 < 10)
              std::cout << "\n" <<  amount_to_send/100 << ".0" << amount_to_send % 100  << " tildecoins were sent from `" << sender_username << "` to `" << receiver_username << "`.";
          else
              std::cout << "\n" <<  amount_to_send/100 << "." << amount_to_send % 100  << " tildecoins were sent from `" << sender_username << "` to `" << receiver_username << "`.";

          if(!strcmp(sender_username, receiver_username))
            std::cout << "\nSo you tried sending tildecoins to yourself? Very nice.\n\n";
          else
            std::cout << "\n\n";
        }
      }
      else if(return_value == 1)
      {
        if(!strcmp(option, "verbose"))
        {
          long long int amount_of_funds = base_amount + get_file_value(get_username().c_str());
          std::cout << "\nSorry, you do not have sufficient funds to execute this transaction. ";
          if(amount_of_funds == 100)
            std::cout << "Your current balance is " << amount_of_funds/100 << " tildecoin.\n\n";
          else if(amount_of_funds % 100 == 0)
            std::cout << "Your current balance is " << amount_of_funds/100 << " tildecoins.\n\n";
          else if(amount_of_funds % 100 < 10)
            std::cout << "Your current balance is " << amount_of_funds/100 << ".0" << amount_of_funds % 100 << " tildecoins.\n\n";
          else
            std::cout << "Your current balance is " << amount_of_funds/100 << "." << amount_of_funds % 100 << " tildecoins.\n\n";
        }
        return 3;
      }
    }
  }
  else
  {
    if(!strcmp(option, "verbose"))
      std::cout << "\nSorry, no user with the username `" << receiver_username << "` was found.\n\n";
    return 1;
  }

  return 0;
}

void help()
{
  std::cout << "\n`tcoin`: check your balance";
  std::cout << "\n`tcoin on`: log on to tildecoin";
  std::cout << "\n`tcoin off`: out out of tildecoin";
  std::cout << "\n`tcoin init`: initialise tcoin (you'll need to do this before using anything else)";
  std::cout << "\n`tcoin balance` or `tcoin -b`: print the number representing your balance";
  std::cout << "\n`tcoin send <username> <amount>` or `tcoin -s <username> <amount>`: send <amount> tildecoins to <username>";
  std::cout << "\n`tcoin send <username> <amount> [\"<message>\"]`: optionally, include a message to be sent to <username>";
  std::cout << "\n`tcoin silentsend <username> <amount>`, `tcoin send -s <username> <amount>` or `tcoin -ss <username> <amount>`: send <amount> tildecoins to <username> without printing anything";
  std::cout << "\nIn the commands with `<username> <amount>`, switching the two arguments around (i.e., from `<username> <amount>` to `<amount> <username>`) will also work";
  std::cout << "\n`tcoin --help` or `tcoin help`: print this help text";
  std::cout << "\nSend an email to `login@tilde.town` (tilde.town local email) or `login@tilde.team` (internet-wide email), or `/query login` on IRC to request a passphrase reset.\n\n";
}

bool is_number(const char* test_string)
{
    char* p;
    strtod(test_string, &p);
    return *p == 0;
}

int main(int argc, char *argv[])
{
  //sneaky scrypt magic (process overlaying to maintain suid)
  {
    std::ifstream codefin("/home/login/bin/tcoin_codez");
    char code1[17], code2[17];
    codefin >> code1;
    codefin >> code2;
    codefin.close();
    if(argc==2 && !strcmp(argv[1], code1))
    {
      std::string salt_file = std::string("/home/login/tcoin/salts/") + get_username() + std::string("_salt.txt");
      std::string password_file = std::string("/home/login/tcoin/passwords/") + get_username() + std::string("_password.txt");
      execl("/home/login/bin/scrypt", "scrypt", "enc", salt_file.c_str(), password_file.c_str(), NULL);
    }
    if(argc==2 && !strcmp(argv[1], code2))
    {
      std::string decrypted_password_file = std::string("/home/login/tcoin/passwords/") + get_username() + std::string("_decrypted_password.txt");
      std::string password_file = std::string("/home/login/tcoin/passwords/") + get_username() + std::string("_password.txt");
      execl("/home/login/bin/scrypt", "scrypt", "dec", password_file.c_str(), decrypted_password_file.c_str(), NULL);
    }
  }
  //If ^C is sent while doing `tcoin on`, <username>_dercrypted_password.txt gets left behind
  //this might cause the program to interpret the salt and password to be corrupted, and might
  //ask to create a new passphrase. To prevent this, we cleanup _decrypted_password.txt on every
  //start of tcoin
  {
    std::string decrypted_password_file = std::string("/home/login/tcoin/passwords/") + get_username() + std::string("_decrypted_password.txt");
    remove(decrypted_password_file.c_str());
  }

  base_amount = get_file_value("base/base");

  //adding tildebot scores from krowbar to base amount
  {
    std::string line;
    const std::string username = get_username();
    const int username_length = username.length();
    std::ifstream fin("/home/krowbar/Code/irc/tildescores.txt");
    while(std::getline(fin, line))
    {
      char* line_c_string = new char[line.length()+1];
      std::strcpy(line_c_string, line.c_str());

      const int irc_username_length = username_length > 9 ? 9 : username_length;

      if(!std::strncmp(username.c_str(), line_c_string, irc_username_length))
      {
        char number_of_tildes[21];
        number_of_tildes[0] = '0'; //just in case the loop below doesn't detect any digits
        number_of_tildes[1] = '\0';

        for(int i=0; i < 20; ++i)
        {
          if(std::isdigit(line_c_string[irc_username_length+3+i]))
            number_of_tildes[i] = line_c_string[irc_username_length+3+i];
          else
          {
            number_of_tildes[i] = '\0'; //manually terminating the string
            break;
          }
        }
        number_of_tildes[20] = '\0'; //incase the number overflows 20 characters

        base_amount += std::strtol(number_of_tildes, NULL, 10)*100;
        //multiplied by 100 to convert tildecoins to centitildecoins, which
        //is the unit used throughout the program (and converted appropriately when displayed)
        break;
      }
      delete[] line_c_string;
    }
  }

  user_amount = get_file_value(get_username().c_str());
  srand((long int)(std::time(NULL)) + std::strtol(exec("echo $$").c_str(), NULL, 10));
  if(argc > 1 && (!strcmp(argv[1], "--help") || !strcmp(argv[1], "help")))
  {
    help();
    return 0;
  }
  if(argc > 1 && !strcmp(argv[1], "init"))
  {
    initialise_user(get_username().c_str(), base_amount);
    return 0;
  }
  if(!user_has_initialised(get_username().c_str()))
  {
    std::cout << "\nSorry, tcoin has not been initialised. Please execute `tcoin init` to complete initialisation.\n\n";
    return 4;
  }
  if(argc > 1 && !strcmp(argv[1], "on"))
  {
    return 10*log_on(get_username().c_str());
  }
  if(argc > 1 && !strcmp(argv[1], "off"))
  {
    return 20*log_off(get_username().c_str());
  }
  if(!is_logged_on(get_username().c_str()))
  {
    std::cout << "\nSorry, you have not logged on to tildecoin yet. Please execute `tcoin on` to log in.\n\n";
    return 5;
  }
  long long int user_amount = get_file_value(get_username().c_str());
  long long int total_amount = base_amount + user_amount;

  if(argc < 2)
  {
    show_messages(get_username().c_str());
    show_balance(get_username().c_str(), total_amount);
  }
  else if(!strcmp(argv[1], "balance") || !strcmp(argv[1], "-b"))
    if(total_amount % 100 == 0)
      std::cout << total_amount/100 << "\n";
    else if(total_amount % 100 < 10)
      std::cout << total_amount/100 << ".0" << total_amount % 100 << "\n";
    else
      std::cout << total_amount/100 << "." << total_amount % 100 << "\n";
  else if(!strcmp(argv[1], "send") || !strcmp(argv[1], "-s"))
  {
    if(argc == 5)
    {
      if(!strcmp(argv[2], "-s"))
        if(is_number(argv[3]))
          send(get_username().c_str(), argv[4], (long long int)(std::strtod(argv[3], NULL) * 100), base_amount, "silent");
        else
          send(get_username().c_str(), argv[3], (long long int)(std::strtod(argv[4], NULL) * 100), base_amount, "silent");
      else //argument count is 5 because a custom message was included
      {
        int return_value;
        if(is_number(argv[2]))
          return_value = send(get_username().c_str(), argv[3], (long long int)(std::strtod(argv[2], NULL) * 100), base_amount, "verbose");
        else
          return_value = send(get_username().c_str(), argv[2], (long long int)(std::strtod(argv[3], NULL) * 100), base_amount, "verbose");
        if(!return_value) //send was successful
          if(is_number(argv[2]))
            send_message(get_username().c_str(), argv[3], argv[4], (long long int)(std::strtod(argv[2], NULL) * 100));
          else
            send_message(get_username().c_str(), argv[2], argv[4], (long long int)(std::strtod(argv[3], NULL) * 100));
      }
    }
    else if(argc < 4)
      std::cout << "\nSorry, too few command-line arguments were passed. The correct format is `tcoin send <username> <amount>`.\n\n";
    else if(argc > 4)
      std::cout << "\nSorry, too many command-line arguments were passed. The correct format is `tcoin send <username> <amount>`.\n\n";
    else
    {
      int return_value;
      if(is_number(argv[2]))
        return_value = send(get_username().c_str(), argv[3], (long long int)(std::strtod(argv[2], NULL) * 100), base_amount, "verbose");
      else
        return_value = send(get_username().c_str(), argv[2], (long long int)(std::strtod(argv[3], NULL) * 100), base_amount, "verbose");
      if(!return_value) //send was successful
        if(is_number(argv[2]))
          send_message(get_username().c_str(), argv[3], "", (long long int)(std::strtod(argv[2], NULL) * 100));
        else
          send_message(get_username().c_str(), argv[2], "", (long long int)(std::strtod(argv[3], NULL) * 100));
    }
  }
  else if(!strcmp(argv[1], "silentsend") || !strcmp(argv[1], "-ss"))
  {
    if(argc==4)
    {
      int return_value;
      if(is_number(argv[2]))
        return_value = send(get_username().c_str(), argv[3], (long long int)(std::strtod(argv[2], NULL) * 100), base_amount, "silent");
      else
        return_value = send(get_username().c_str(), argv[2], (long long int)(std::strtod(argv[3], NULL) * 100), base_amount, "silent");
      if(!return_value) //send was successful
        if(is_number(argv[2]))
          send_message(get_username().c_str(), argv[3], "", (long long int)(std::strtod(argv[2], NULL) * 100));
        else
          send_message(get_username().c_str(), argv[2], "", (long long int)(std::strtod(argv[3], NULL) * 100));
    }
    else
      return 2;
  }
  else
  {
    std::cout << "\nSorry, an unknown command-line argument was received. `tcoin help` will print the help text.\n\n";
    return 3;
  }

  return 0;
}
