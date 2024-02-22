#include <chrono>
#include <functional>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/int32_multi_array.hpp"


using namespace std::chrono_literals;
using std::placeholders::_1;


class merge_arrays_node : public rclcpp::Node
{
  public:
  merge_arrays_node() 
  : Node("merge_arrays_node")
  {
    //We are outputting to /output/array
    publisher_ = this->create_publisher<std_msgs::msg::Int32MultiArray>("/output/array", 10);\
    //keep track of /input/array1 and /input/array2
    array1_subscription_ = this->create_subscription<std_msgs::msg::Int32MultiArray>(
    "/input/array1", 10, std::bind(&merge_arrays_node::array1_callback, this, _1));
    array2_subscription_ = this->create_subscription<std_msgs::msg::Int32MultiArray>(
    "/input/array2", 10, std::bind(&merge_arrays_node::array2_callback, this, _1));
  }
  private:
    void array1_callback(const std_msgs::msg::Int32MultiArray & arr){
      array1 = arr.data;
      merge();
    }
    void array2_callback(const std_msgs::msg::Int32MultiArray & arr){
      array2 = arr.data;
      merge();
    }
    void merge()
    {
      //if haven't recieved data from either array1 or 2 yet, return prematurely.
      if (array1.empty() || array2.empty()){
        return;
      }

      //basic logic to put numbers together in sequential order.
      unsigned long int i = 0;
      unsigned long int j = 0;
      std::vector<int> output;
      while (i < array1.size() && j < array2.size())
      {
        if (array1[i] < array2[j]){
          output.push_back(array1[i]);
          i++;
        } else {
          output.push_back(array2[j]);
          j++;
        }
      }
      //push the remainders on
      while (i < array1.size()){
        output.push_back(array1[i]);
        i++;
      }
      while (j < array2.size()){
        output.push_back(array2[j]);
        j++;
      }
      //store vector in a Int34MuliArray msg
      std_msgs::msg::Int32MultiArray msg_out;
      msg_out.data = output;
      //log it in terminal so i know im not going insane
      std::stringstream ss;
      std::copy(msg_out.data.begin(), msg_out.data.end(), std::ostream_iterator<int>(ss, " "));
      std::string publish_msg = "Publishing: [ " + ss.str() + "]";
      RCLCPP_INFO(this->get_logger(), publish_msg.c_str());
      
      //actually publish it (i hope this works)
      publisher_->publish(msg_out);

      //clear the "used" arrays when done (mostly to clean up the logger so i know its working properly easier)
      array1.clear();
      array2.clear();
    }

    //publisher
    rclcpp::Publisher<std_msgs::msg::Int32MultiArray>::SharedPtr publisher_;

    //Subscriptions
    rclcpp::Subscription<std_msgs::msg::Int32MultiArray>::SharedPtr array1_subscription_;
    rclcpp::Subscription<std_msgs::msg::Int32MultiArray>::SharedPtr array2_subscription_;

    //Variables to hold last recieved arrays
    std::vector<int32_t> array1;
    std::vector<int32_t> array2;   
};


int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<merge_arrays_node>());
  rclcpp::shutdown();
  return 0;
}

