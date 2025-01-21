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
                 population_size: int = 3,
                 learn_rate: float = 0.1,
                 neg_learn_rate: float = 0.075,
                 mut_prob: float = 0.02,
                 mut_shift: float = 0.05):
        self.parameters = parameters
        self.population_size = population_size
        self.total_bits = len(parameters) * 16  # 16 bits per parameter
        self.prob_vector = np.full(self.total_bits, 0.5)
        self.learn_rate = learn_rate
        self.neg_learn_rate = neg_learn_rate
        self.mut_prob = mut_prob
        self.mut_shift = mut_shift
        
    def generate_population(self) -> List[List[bool]]:
        """Generate a population of binary strings based on probability vector"""
        population = []
        for _ in range(self.population_size):
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
        
    def create_engine_config(self, params: Dict[str, float], idx: int) -> List[str]:
        """Create engine configuration with given parameters"""
        config = [
            "-engine",
            f"name=Eng{idx}",
            f"cmd={self.engine_path}"
        ]
        
        # Add parameters as options if they exist
        if params:
            for name, value in params.items():
                config.append(f"option.{name}={int(value)}")
        
        return config

    def run_tournament(self, configs: List[Dict[str, float]], 
                      games_per_encounter: int = 10) -> List[float]:
        """Run a tournament between different engine configurations"""
        
        # Base command with required options
        cmd = [
            self.cutechess_cli,
            "-each", "tc=40/20+0.01", "proto=uci",
            "-tournament", "gauntlet",
            "-games", str(games_per_encounter),
            "-concurrency", "4",
            "-recover"
        ]
        
        # Create first engine (baseline) without modifications
        cmd.extend(self.create_engine_config({}, 0))
        
        # Add all variant configurations
        for i, config in enumerate(configs, start=1):
            cmd.extend(self.create_engine_config(config, i))
            
        print("Executing command:", " ".join(cmd))
            
        try:
            result = subprocess.run(cmd, capture_output=True, text=True)
            print("Command output:", result.stdout)
            print("Command errors:", result.stderr)
            
            if result.returncode != 0:
                print(f"Tournament failed with return code {result.returncode}")
                return [0.0] * len(configs)
                
            return self.parse_tournament_results(result.stdout, len(configs))
            
        except subprocess.CalledProcessError as e:
            print(f"Tournament failed with error: {e}")
            print(f"Error output: {e.stderr}")
            return [0.0] * len(configs)
        except Exception as e:
            print(f"Unexpected error running tournament: {e}")
            return [0.0] * len(configs)
    
    def parse_tournament_results(self, output: str, num_variants: int) -> List[float]:
        """Parse tournament results to get win percentages for variant engines only"""
        lines = output.strip().split('\n')
        
        # Find the rankings section
        rankings_start = -1
        for i, line in enumerate(lines):
            if line.strip().startswith("Rank Name"):
                rankings_start = i + 1
                break
                
        if rankings_start == -1:
            print("Error: Could not find rankings in tournament output")
            print("Complete output:", output)
            return [0.0] * num_variants
            
        # Parse each engine's score
        scores = []
        i = rankings_start
        engine_scores = {}  # Map engine number to score
        
        while i < len(lines) and lines[i].strip() and not lines[i].startswith("SPRT"):
            line = lines[i].strip()
            print(f"Parsing line: {line}")
            
            try:
                parts = line.split()
                engine_num = int(parts[1][3:])  # Extract number from "EngX"
                score = float(parts[-2].rstrip('%')) / 100.0
                if engine_num != 0:  # Skip baseline engine
                    engine_scores[engine_num] = score
            except (IndexError, ValueError) as e:
                print(f"Error parsing score from line: {line}")
                print(f"Error: {e}")
            i += 1
            
        # Convert to list of scores in original order
        scores = [engine_scores.get(i, 0.0) for i in range(1, num_variants + 1)]
            
        return scores

def main():
    # Define parameters with more restricted ranges
    parameters = [
        # Core piece values - keep relatively close to standard values
        Parameter("PawnValue", 80, 120, True),
        Parameter("KnightValue", 280, 360, True),
        Parameter("BishopValue", 290, 370, True), 
        Parameter("RookValue", 450, 550, True),
        Parameter("QueenValue", 850, 950, True),
        Parameter("KingValue", 1800, 2200, True),
        
        # Position bonuses - keep moderate to avoid extreme positioning
        Parameter("PawnPositionBonus", 30, 70, True),
        Parameter("KnightPositionBonus", 30, 70, True),
        Parameter("BishopPositionBonus", 30, 70, True),
        Parameter("RookPositionBonus", 30, 70, True),
        Parameter("QueenPositionBonus", 30, 70, True),
        Parameter("KingPositionBonus", 30, 70, True),
        
        # Tactical bonuses - reduced ranges to prevent excessive values
        Parameter("MateScore", 50000, 150000, True),
        Parameter("SupportedPawnBonus", 50, 100, True),
        Parameter("SupportingPawnBonus", 65, 115, True),
        Parameter("SupportingPieceBonus", 75, 125, True),
        
        # Penalties - keep moderate
        Parameter("DoubledPawnPenalty", -100, -40, True),
        Parameter("IsolatedPawnPenalty", -100, -60, True),
        Parameter("CheckedPenalty", -1500, -500, True),
        
        # Strategic bonuses - reduced ranges
        Parameter("CheckingBonus", 1000, 2000, True),
        Parameter("BishopPairBonus", 75, 175, True),
        Parameter("RookOpenFileBonus", 150, 350, True)
    ]
    
    # Initialize PBIL with smaller population
    pbil = PBIL(parameters, population_size=3)
    evaluator = EngineEvaluator(
        "D:/dev/SoftDevProjects/GitHub/KnightEngine/meta/CuteChess/cutechess-cli.exe",
        "D:/dev/SoftDevProjects/GitHub/KnightEngine/stored_engines/MaterialEngine_v0_72_R.exe"
    )
    
    # Open results file
    with open("tuning_results.txt", "w") as f:
        f.write("Generation,Best Parameters,Win Rate\n")
    
    # Main optimization loop
    for generation in range(100):  # 100 generations
        print(f"\nGeneration {generation}")
        
        # Generate population
        population = pbil.generate_population()
        
        # Convert binary individuals to parameter values
        configurations = [pbil.decode_individual(ind) for ind in population]
        
        # Run tournament
        fitness_scores = evaluator.run_tournament(configurations)
        
        if not fitness_scores or len(fitness_scores) == 0:
            print("No valid fitness scores returned from tournament")
            continue
            
        # Find best and worst performing variants
        print(f"\nBest idx: {np.argmax(fitness_scores)}")
        sorted_indices = np.argsort(fitness_scores)[::-1]  # Sort in descending order
        best_idx = sorted_indices[0]
        worst_idx = sorted_indices[-1]
        
        print("\nTournament Results:")
        for i, score in enumerate(fitness_scores):
            print(f"{i+1}. Engine {i+1}: {score*100:.1f}%")
        
        # Update probability vector if we have valid indices
        if best_idx < len(population) and worst_idx < len(population):
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