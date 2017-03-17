all: 
	gcc -g -o otp_enc_d otp_enc_d.c 
	gcc -g -o otp_enc otp_enc.c
	gcc -g -o otp_dec_d otp_dec_d.c
	gcc -g -o otp_dec otp_dec.c
	gcc -g -o keygen keygen.c
clean:
	rm -rf otp_enc_d otp_enc otp_dec_d otp_dec keygen
