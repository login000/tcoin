# tcoin
Currency simulator on tilde.town and other tildeboxes on tildeverse.org

## Instructions to get started
1. Clone the repo.
2. Create a service user specifically for tcoin.
3. Make sure you have `bash`, `realpath`, `echo`, `touch`, `mkdir`, `g++`, `head`, `true`, `base64`, and `cat` are available.
4. Run `./ntcoin a b c d` as the service user, where `a`, `b`, `c` and `d` are defined as follows:
    * `a`: the absolute path to the directory (without the trailing slash) where the `tcoin` folder containing all the users' data will be stored (i.e., the script will create the directory `a/tcoin/`)
    * `b`: the absolute path to the directory (without the trailing slash) where the `tcoin` and `pcoin` executables will be located, i.e., all users will have to have `r-x` permissions on `b` so that they can access the executables `b/tcoin` and `b/pcoin`. Please note that `b/tcoin` and `b/pcoin` will have the suid bit set. This means that when they run `b/tcoin` and `b/pcoin`, it will be with the uid of the service user (not their own uid). This is how they are able to send and receive coins only through the executables but not modify the `a/tcoin/` themselves.
    * `c`: an integer number of tildecoins that every user starts with. This is stored in `a/tcoin/base/base.txt`, and for universal-basic-income-style inflation, one can modify `a/tcoin/base/base.txt` using a cronjob. This will increase everybody's coins in real time.
    * `d`: a host name (in quotation marks, e.g., d = "tilde.town") that will appear in `tcoin --help`.
## Instructions to create program accounts
1. Run `b/npcoin a` as the service user in the parent directory of the `tcoin` folder, i.e., if the `tcoin` folder is in directory with absolute path `x`, then run `cd x; b/npcoin a`. Here, `a` is the name of the program account you want to create, and `b` is the absolute path to the `tcoin` repository that contains the `npcoin` script. Program accounts should start with a capital letter (e.g., `My_program_account` or `My-program-account`).
2. Create a file in the `tcoin/secrets/pcoin_keys/` directory with the following name: `My_program_account.txt` (assuming the name of the program is `My_program_account`) and add the following newline-terminated line to it:
`<a 64-character random alphanumeric (either all lowercase, or mixed-case will also work) string>`
3. The string will be the key that the program uses to access and operate its account, and must be given to the requester of the program account securely (and currently, manually). I usually use random.org to generate these 64-character strings because I don't trust pseudorandom number generators (even if they are cryptographically-secure pseudorandom number generators). Anybody who can guess a key can use that program's account.
4. Program-account creation is currently not automated because of concerns about name-squatting and similarly-named program names.
