package preprocess;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import data.Skeleton;

/**
* This class is used to process skeleton data into some spatial information.
*
* @author  WeiChun
*/
public class SkeletonInfo {
	
	/**
	* process joint positions into distance
	*   
	* @param: list of Skeleton objects
	* 
	* @return: arrays of displacements in each axis
	*/
	public static double[][] getDistanceList(ArrayList<Skeleton> jointspos) {
		
		double [][] distance = new double[3][jointspos.size()/6*5];
		int len = 0;
		
		for (int i = 0; i < jointspos.size(); i+=6) {
			if (i+6 > jointspos.size()) {
				break;
			}
			List<Skeleton>subjointspos = jointspos.subList(i, i+6);
			for (int j = 0; j < 5; j++) {
				distance[0][len] = subjointspos.get(j+1).getRight_wrist()[0] - subjointspos.get(j).getRight_wrist()[0];
				distance[1][len] = subjointspos.get(j+1).getRight_wrist()[1] - subjointspos.get(j).getRight_wrist()[1];
				distance[2][len] = subjointspos.get(j+1).getRight_wrist()[2] - subjointspos.get(j).getRight_wrist()[2];
				len++;
			}
		}
		
		return distance;
		
	}
	
	/**
	* process joint positions into velocity
	*   
	* @param: list of Skeleton objects
	* 
	* @return: arrays of velocity in each axis
	*/
	public static double[][] getSpeedList(ArrayList<Skeleton> jointspos) {
		
		double [][] distance = getDistanceList(jointspos);
		double [][] speed = new double[3][distance[0].length];
		
		for (int i = 0; i < distance[0].length; i++) {
			for (int axis = 0; axis < 3; axis++) {
				speed[axis][i] = distance[axis][i] * 30;
			}
		}
		
		return speed;
		
	}
	
	/**
	* process joint positions into acceleration
	*   
	* @param: list of Skeleton objects
	* 
	* @return: arrays of acceleration in each axis
	*/
	public static double[][] getAccList(ArrayList<Skeleton> jointspos) {
		
		double[][] speed = getSpeedList(jointspos);
		double[][] acc = new double[3][speed[0].length/5*4];
		int len = 0;
		
		for (int i = 0; i < speed[0].length; i += 5) {
			if(i+5 > speed[0].length) {
				break;
			}
			double[] subspeed_x = Arrays.copyOfRange(speed[0], i, i + 5);
			double[] subspeed_y = Arrays.copyOfRange(speed[1], i, i + 5);
			double[] subspeed_z = Arrays.copyOfRange(speed[2], i, i + 5);
			for (int j = 0; j < 4; j++) {
				acc[0][len] = (subspeed_x[j + 1] - subspeed_x[j]) * 30;
				acc[1][len] = (subspeed_y[j + 1] - subspeed_y[j]) * 30;
				acc[2][len] = (subspeed_z[j + 1] - subspeed_z[j]) * 30;
				len++;
			}
		}
		
		return acc;
		
	}
	
}
