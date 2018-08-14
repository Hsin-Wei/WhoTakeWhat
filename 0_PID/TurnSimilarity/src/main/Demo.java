package main;

import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;

import com.opencsv.CSVReader;
import com.opencsv.CSVWriter;

import data.HeadPos;
import data.Inertia;
import data.Skeleton;
import data.TurnList;
import preprocess.BodyExtraction;
import preprocess.ProcessTool;
import preprocess.ReadData;
import preprocess.TurnMag;
import scoring.FusionAlgo;

public class Demo {

	public static void main(String[] args) {
		String rootDir = "C:/Users/NCTU/Desktop/WhoTakeWhat/data/";
		String[] users = { "Hsin", "Danny"};

		// separate different users in VSFile
		ArrayList<Integer> skeletonIDs;
		skeletonIDs = BodyExtraction.bodyCount(rootDir + "/Skeleton/VSFile.csv");
		System.out.println("Skeleton count:" + skeletonIDs.size());

		for (int i = 0; i < skeletonIDs.size(); i++) {
			BodyExtraction.bodyWriter(rootDir + "/Skeleton/VSFile.csv", rootDir + "/Skeleton/VSFile_" + i + ".csv",
					skeletonIDs.get(i));
		}

		// transform inertial data file from .txt to .csv
		// for (int i = 0; i < users.length; i++) {
		// ProcessTool.reformat(rootDir + "/IMU/testing/" + users[i] + ".txt", rootDir +
		// "/IMUData/OralDefense/" + users[i] + ".csv");
		// }

		// read skeleton data and inertial data of each person
		ArrayList<ArrayList<Skeleton>> skeletons = new ArrayList<ArrayList<Skeleton>>();
		ArrayList<ArrayList<Inertia>> inertia_set = new ArrayList<ArrayList<Inertia>>();
		ArrayList<ArrayList<HeadPos>> bodyHead = new ArrayList<ArrayList<HeadPos>>();

		for (int i = 0; i < skeletonIDs.size(); i++) {
			ArrayList<Skeleton> jointspos = ReadData.readKinect_smooth(rootDir + "/Skeleton/VSFile_" + i + ".csv");
			ArrayList<HeadPos> HeadXY = ReadData.readHead(rootDir + "/Skeleton/VSFile_" + i + ".csv");
			skeletons.add(jointspos);
			bodyHead.add(HeadXY);
		}

		for (int i = 0; i < users.length; i++) {
			// synchronize starter
			int offset = ProcessTool.getStarter(rootDir + "/Skeleton/VSFile.csv",
					rootDir + "/IMU/forPID/" + users[i] + ".csv");
			ArrayList<Inertia> inertia = ReadData.readIMU(rootDir + "/IMU/forPID/" + users[i] + ".csv");
			inertia = new ArrayList<Inertia>(inertia.subList(offset, inertia.size()));
			inertia_set.add(inertia);
		}

		int[] skeletonSizes = new int[skeletons.size()];
		for (int i=0; i<skeletons.size(); i++) {
			skeletonSizes[i] = skeletons.get(i).size();
		}
		Arrays.sort(skeletonSizes);
		int minSize = skeletonSizes[0];
		
		// pair skeleton data with users' IDs every 10 seconds
		for (int t = 0; t < minSize / 300; t++) {
			ArrayList<Double> scores = new ArrayList<Double>();
			for (int i = 0; i < skeletons.size(); i++) {
				for (int j = 0; j < inertia_set.size(); j++) {
					ArrayList<Skeleton> sub_jointspos = new ArrayList<Skeleton>(
							skeletons.get(i).subList(t * 200, (t + 1) * 200)); // 200 samples in 10 seconds
					ArrayList<Inertia> sub_inertia = new ArrayList<Inertia>(
							inertia_set.get(j).subList(t * 1000, (t + 1) * 1000)); // 1000 samples in 10 seconds

					TurnList kinectTurns = TurnMag.genKINECTTurnList(sub_jointspos);
					TurnList imuTurns = TurnMag.genIMUTurnList(sub_inertia);
					scores.add(FusionAlgo.calResult_alg3(kinectTurns, imuTurns));
				}
			}
			int[][] result = IDPairing.pairing(scores, skeletonIDs.size(), users.length);

			// write pairing results of each frame every 10 seconds
			int[] match = new int[skeletonIDs.size()];
			for (int i = 0; i < skeletonIDs.size(); i++) {
				match[i] = -1;
			}
			for (int i = 0; i < skeletons.size(); i++) {
				for (int j = 0; j < inertia_set.size(); j++) {
					System.out.printf("%d ", result[i][j]);
					if (result[i][j] == 1) {
						match[i] = j;
					}
				}
				System.out.println("");
			}
			System.out.println("=============");

			
			ArrayList<String> timestamp = new ArrayList<String>();
			CSVReader cr = null;
			try {
				cr = new CSVReader(new FileReader("C:/Users/NCTU/Desktop/WhoTakeWhat/data/Skeleton/frameTime.csv"));
				String[] line;
				while ((line = cr.readNext()) != null) {
					timestamp.add(line[1]);
				}
				cr.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
			
			
			String[] IDCoordinate = new String[skeletonIDs.size() * 6 + 1];
			CSVWriter cw;
			try {
				cw = new CSVWriter(new FileWriter(rootDir + "result/PID/result.csv", true), ',',
						CSVWriter.NO_QUOTE_CHARACTER);
				for (int k = t * 300; k < (t + 1) * 300; k++) {
					for (int i = 0; i < skeletonIDs.size(); i++) {
						if (match[i] >= 0) {
							IDCoordinate[i * 6] = users[match[i]];
						} else {
							IDCoordinate[i * 6] = "Unknown";
						}
						IDCoordinate[i * 6 + 1] = String.valueOf(bodyHead.get(i).get(k).getX());
						IDCoordinate[i * 6 + 2] = String.valueOf(bodyHead.get(i).get(k).getY());
						IDCoordinate[i * 6 + 3] = String.valueOf(skeletons.get(i).get(k).getRight_wrist()[0]);
						IDCoordinate[i * 6 + 4] = String.valueOf(skeletons.get(i).get(k).getRight_wrist()[1]);
						IDCoordinate[i * 6 + 5] = String.valueOf(skeletons.get(i).get(k).getRight_wrist()[2]);
						
					}
					
					IDCoordinate[IDCoordinate.length-1] = timestamp.get(k);
					cw.writeNext(IDCoordinate);
				}
				cw.close();
			} catch (IOException e) {
				e.printStackTrace();
			}

		}

	}

}
