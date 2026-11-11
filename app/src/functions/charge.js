
const PAYSTACK_SECRET_KEY = process.env.PAYSTACK_SECRET_KEY;

import https from "https";


// Generate a one-time Virtual Bank Account for Bank Transfer
async function requestFunding(gmail = "test1@gmail.com", amount = 500, command) {
    const params = JSON.stringify({
        email: gmail,
        amount: amount * 100, // amount in kobo
        bank_transfer: {
            account_expires_at: new Date(Date.now() + 30 * 60 * 1000).toISOString() // Optional: expires in 30 mins
        },
        metadata: {
            guide: {
                command,
                email: gmail,
                amount: amount // amount in naira
            }
        }
    });

    const options = {
        hostname: "api.paystack.co",
        port: 443,
        path: "/charge",
        method: "POST",
        headers: {
            Authorization: `Bearer ${PAYSTACK_SECRET_KEY}`,
            "Content-Type": "application/json"
        }
    };

    return new Promise((resolve, reject) => {
        const req = https.request(options, res => {
            let data = "";

            res.on("data", chunk => {
                data += chunk;
            });

            res.on("end", () => {
                try {
                    const fundingRes = JSON.parse(data);

                    if (!fundingRes.status) {
                        console.log("Paystack error response:", fundingRes);
                        return reject(new Error(fundingRes.message));
                    }

                    resolve(fundingRes);
                } catch (error) {
                    reject(new Error(`Failed to parse response: ${error.message}`));
                }
            });
        });

        req.on("error", error => {
            console.error("Request error:", error);
            reject(error);
        });

        req.write(params);
        req.end();
    });
}

export async function chargeUser(gmail, amount, command, channel = "paystack") {
    if (channel !== "paystack") {
        throw new Error(`${channel} is not a supported channel`);
    }

    const paymentRes = await requestFunding(gmail, amount, command);
    console.log("Payment response:", `User is charged successfully. Reference: ${paymentRes.data.reference}, Account Number: ${paymentRes.data.account_number}, Bank Name: ${paymentRes.data.bank.name}, Account Name: ${paymentRes.data.account_name}, Expires At: ${paymentRes.data.account_expires_at}`);
    // Extract virtual bank account details from paymentRes.data
    return {
        reference: paymentRes.data.reference,
        account_number: paymentRes.data.account_number,
        bank_name: paymentRes.data.bank.name,
        account_name: paymentRes.data.account_name,
        expires_at: paymentRes.data.account_expires_at
    };
}


