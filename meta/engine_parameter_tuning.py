import subprocess
import random
import json
import numpy as np
from dataclasses import dataclass
from typing import List, Dict, Any
import os

@dataclass
class Parameter:
    """Represents a tunable parameter with its constraints"""
    name: str
    min_value: float
    max_value: float
    is_integer: bool = False

    def encode(self, value: float) -> List[bool]:
        """Encode a parameter value into binary"""
        # Use 16 bits for each parameter
        normalized = (value - self.min_value) / (self.max_value - self.min_value)
        binary = format(int(normalized * 65535), '016b')
        return [bit == '1' for bit in binary]

    def decode(self, bits: List[bool]) -> float:
        """Decode binary back into a parameter value"""
        binary_str = ''.join('1' if bit else '0' for bit in bits)
        normalized = int(binary_str, 2) / 65535
        value = self.min_value + normalized * (self.max_value - self.min_value)
        return round(value) if self.is_integer else value

class PBIL:
    def __init__(self, parameters: List[Parameter], 
                 learn_rate: float = 0.1,
                 neg_learn_rate: float = 0.075,
                 mut_prob: float = 0.02,
                 mut_shift: float = 0.05):
        self.parameters = parameters
        self.total_bits = len(parameters) * 16  # 16 bits per parameter
        self.prob_vector = np.full(self.total_bits, 0.5)
        self.learn_rate = learn_rate
        self.neg_learn_rate = neg_learn_rate
        self.mut_prob = mut_prob
        self.mut_shift = mut_shift
        
    def generate_population(self, size: int) -> List[List[bool]]:
        """Generate a population of binary strings based on probability vector"""
        population = []
        for _ in range(size):
            individual = [random.random() < p for p in self.prob_vector]
            population.append(individual)
        return population
    
    def decode_individual(self, individual: List[bool]) -> Dict[str, float]:
        """Convert binary individual back into parameter values"""
        values = {}
        for i, param in enumerate(self.parameters):
            start = i * 16
            param_bits = individual[start:start + 16]
            values[param.name] = param.decode(param_bits)
        return values
    
    def update_probabilities(self, best_individual: List[bool], 
                           worst_individual: List[bool]):
        """Update probability vector based on best and worst individuals"""
        for i in range(self.total_bits):
            if best_individual[i] == worst_individual[i]:
                # Both best and worst agree - regular update
                self.prob_vector[i] = (self.prob_vector[i] * (1 - self.learn_rate) + 
                                     (1.0 if best_individual[i] else 0.0) * self.learn_rate)
            else:
                # Best and worst disagree - stronger update
                learn_rate2 = self.learn_rate + self.neg_learn_rate
                self.prob_vector[i] = (self.prob_vector[i] * (1 - learn_rate2) +
                                     (1.0 if best_individual[i] else 0.0) * learn_rate2)
        
        # Mutation
        for i in range(self.total_bits):
            if random.random() < self.mut_prob:
                self.prob_vector[i] = (self.prob_vector[i] * (1 - self.mut_shift) +
                                     (1.0 if random.random() < 0.5 else 0.0) * self.mut_shift)

class EngineEvaluator:
    def __init__(self, cutechess_cli_path: str, engine_path: str):
        self.cutechess_cli = cutechess_cli_path
        self.engine_path = engine_path
        
    def create_engine_config(self, params: Dict[str, float], idx: int) -> str:
        """Create engine configuration with given parameters"""
        config = f"name=Engine{idx} cmd={self.engine_path} "
        for name, value in params.items():
            config += f"option.{name}={value} "
        return config.strip()
    
    def run_tournament(self, configs: List[Dict[str, float]], 
                      games_per_encounter: int = 10) -> List[float]:
        """Run a tournament between different engine configurations"""
        # Create engine definitions
        engine_defs = [self.create_engine_config(params, i) 
                      for i, params in enumerate(configs)]
        
        # Build cutechess-cli command
        cmd = [
            self.cutechess_cli,
            "-tournament", "round-robin",
            "-games", str(games_per_encounter),
            "-concurrency", "4",  # Adjust based on your CPU
            "-recover",
            "-each tc=40/20+0.01 proto=uci"
        ]
        
        # Add all engines
        for engine in engine_defs:
            cmd.extend(["-engine", engine])
            
        # Run tournament
        try:
            result = subprocess.run(cmd, capture_output=True, text=True, check=True)
            # Parse results and return win percentages
            return self.parse_tournament_results(result.stdout)
        except subprocess.CalledProcessError as e:
            print(f"Tournament failed: {e}")
            return [0.0] * len(configs)
    
    def parse_tournament_results(self, output: str) -> List[float]:
        """Parse tournament results to get win percentages.
        Returns a list of win percentages for each engine."""
        lines = output.strip().split('\n')
        
        # Find the rankings section
        rankings_start = -1
        for i, line in enumerate(lines):
            if line.startswith("Rank Name"):
                rankings_start = i + 1
                break
                
        if rankings_start == -1:
            print("Error: Could not find rankings in tournament output")
            return []
            
        # Parse each engine's score
        scores = []
        i = rankings_start
        while i < len(lines) and lines[i].strip() and not lines[i].startswith("SPRT"):
            # Split the line and get the score percentage
            parts = lines[i].split()
            try:
                # Score is in the format "XX.X%" - remove the % and convert to float
                score = float(parts[-2].rstrip('%'))
                scores.append(score / 100.0)  # Convert percentage to decimal
            except (IndexError, ValueError) as e:
                print(f"Error parsing score from line: {lines[i]}")
                print(f"Error: {e}")
            i += 1
            
        return scores

def main():
    # Define parameters to tune
    parameters = [
        Parameter("PawnValue", 50, 150, True),
        Parameter("KnightValue", 250, 400, True),
        Parameter("BishopValue", 250, 400, True),
        Parameter("RookValue", 400, 600, True),
        Parameter("QueenValue", 800, 1000, True),
        Parameter("KingValue", 1500, 2000, True),
        Parameter("PawnPositionBonus", 0, 100, True),
        Parameter("KnightPositionBonus", 0, 100, True),
        Parameter("BishopPositionBonus", 0, 100, True),
        Parameter("RookPositionBonus", 0, 100, True),
        Parameter("QueenPositionBonus", 0, 100, True),
        Parameter("KingPositionBonus", 0, 100, True),
        Parameter("MateScore", 5000, 1000000, True),
        Parameter("SupportedPawnBonus", 10, 300, True),
        Parameter("SupportingPawnBonus", 10, 300, True),
        Parameter("SupportingPieceBonus", 10, 300, True),
        Parameter("DoubledPawnPenalty", 0, -200, True),
        Parameter("IsolatedPawnPenalty", 0, -200, True),
        Parameter("CheckedPenalty", -100, -1000, True),
        Parameter("CheckingBonus", 100, 15000, True),
        Parameter("BishopPairBonus", 10, 250, True),
        Parameter("RookOpenFileBonus", 10, 350, True)
    ]
    
    # Initialize PBIL
    pbil = PBIL(parameters)
    evaluator = EngineEvaluator(
        "D:/dev/SoftDevProjects/GitHub/KnightEngine/meta/CuteChess/cutechess-cli.exe",
        "D:/dev/SoftDevProjects/GitHub/KnightEngine/stored_engines/MaterialEngine_v0_71_R.exe"
    )
    
    # Open results file
    with open("tuning_results.txt", "w") as f:
        f.write("Generation,Best Parameters,Win Rate\n")
    
    # Main optimization loop
    for generation in range(100):  # 100 generations
        print(f"Generation {generation}")
        
        # Generate population
        population = pbil.generate_population(2)  # Individuals per generation
        
        # Convert binary individuals to parameter values
        configurations = [pbil.decode_individual(ind) for ind in population]
        
        # Run tournament
        fitness_scores = evaluator.run_tournament(configurations)
        
        # Find best and worst performing engines
        sorted_indices = np.argsort(fitness_scores)[::-1]  # Sort in descending order
        best_idx = sorted_indices[0]
        worst_idx = sorted_indices[-1]
        
        print("\nTournament Results:")
        for i, idx in enumerate(sorted_indices):
            print(f"{i+1}. Engine {idx}: {fitness_scores[idx]*100:.1f}%")
        
        # Update probability vector
        pbil.update_probabilities(population[best_idx], population[worst_idx])
        
        # Save results
        with open("tuning_results.txt", "a") as f:
            result_data = {
                "generation": generation,
                "parameters": configurations[best_idx],
                "score": fitness_scores[best_idx],
                "all_scores": list(fitness_scores)
            }
            f.write(json.dumps(result_data) + "\n")
        
        print(f"Best fitness: {fitness_scores[best_idx]}")
        print(f"Best parameters: {configurations[best_idx]}")

if __name__ == "__main__":
    main()