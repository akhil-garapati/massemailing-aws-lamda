#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/ses/SESClient.h>
#include <aws/ses/model/SendEmailRequest.h>
#include <aws/core/utils/Outcome.h>
#include <aws/core/utils/Stream.h>
#include <iostream>
#include <sstream>
#include <string>

using namespace Aws;
using namespace Aws::S3;
using namespace Aws::S3::Model;
using namespace Aws::SES;
using namespace Aws::SES::Model;

void send_email(const SESClient& ses_client, const Aws::String& sender_email, const Aws::String& recipient_email, const Aws::String& message) {
    SendEmailRequest request;
    request.SetSource(sender_email);

    Message msg;
    msg.SetSubject(Content().WithData("Testing SES").WithCharset("UTF-8"));
    msg.SetBody(Body().WithText(Content().WithData(message).WithCharset("UTF-8")));

    request.SetMessage(msg);
    request.SetDestination(Destination().AddToAddresses(recipient_email));

    auto outcome = ses_client.SendEmail(request);
    if (outcome.IsSuccess()) {
        std::cout << "Email sent to " << recipient_email << std::endl;
    } else {
        std::cerr << "Error sending email: " << outcome.GetError().GetMessage() << std::endl;
    }
}

void lambda_handler(const Aws::String& bucket_name, const Aws::String& object_key) {
    S3Client s3_client;
    SESClient ses_client;

    GetObjectRequest object_request;
    object_request.SetBucket(bucket_name);
    object_request.SetKey(object_key);

    auto get_object_outcome = s3_client.GetObject(object_request);

    if (get_object_outcome.IsSuccess()) {
        std::stringstream ss;
        ss << get_object_outcome.GetResult().GetBody().rdbuf();

        Aws::String csv_data = ss.str().c_str();

        std::istringstream csv_stream(csv_data);
        std::string line;
        while (std::getline(csv_stream, line)) {
            if (line.find("Email,Message") != std::string::npos) continue; // Skip header

            std::istringstream line_stream(line);
            std::string email, message;

            std::getline(line_stream, email, ',');
            std::getline(line_stream, message);

            send_email(ses_client, "firstpirateking.onepiece@gmail.com", email.c_str(), message.c_str());
        }
    } else {
        std::cerr << "Error getting object from S3: " << get_object_outcome.GetError().GetMessage() << std::endl;
    }
}

int main() {
    SDKOptions options;
    InitAPI(options);
    {
        // Replace with actual S3 bucket name and object key
        lambda_handler("your-bucket-name", "your-object-key");
    }
    ShutdownAPI(options);
    return 0;
}
